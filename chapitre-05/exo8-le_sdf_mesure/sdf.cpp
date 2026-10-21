// exo8 - Le SDF, mesure
//
// Le meme panneau "Sortie", deux fois : une fois avec un atlas ordinaire
// (la couverture du glyphe), une fois avec un atlas en mode distance
// signee (la distance au bord de la lettre). On compare de loin, de pres,
// et en memoire.
//
// Fichiers rendus :
//   sdf.cpp            : ce fichier
//   atlas.txt          : les trois tailles d'atlas
//   ordinaire-3m.png   : le panneau ordinaire, vu de trois metres
//   ordinaire-50cm.png : le panneau ordinaire, vu de cinquante centimetres
//   sdf-3m.png         : le panneau SDF, vu de trois metres
//   sdf-50cm.png       : le panneau SDF, vu de cinquante centimetres
//   reponse.txt        : la reponse redigee
//   murs/panneau-ordinaire.png, murs/panneau-sdf.png : les deux images
//     composees sur le processeur (le moteur n'applique pas de nuanceur
//     SDF a une texture : c'est a nous de le faire)
//
// L'atlas est lu par interpolation bilineaire entre les quatre pixels
// voisins : lire au pixel le plus proche donnerait des escaliers dans les
// deux cas, et on ne mesurerait alors que le nearest.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <NKFont/NkFont.h>
#include <NKFont/Embedded/NkFontEmbedded.h>
#include <NKImage/Core/NkImage.h>

#include <NKWindow/NKMain.h>
#include <NKWindow/NKWindow.h>
#include <NKWindow/Core/NkWindowConfig.h>
#include <NKWindow/Core/NkWESystem.h>

#include <NKRHI/Core/NkDeviceFactory.h>
#include <NKRHI/Core/NkDeviceInitInfo.h>
#include <NKRHI/Core/NkIDevice.h>

#include <NKRenderer/NkRenderer.h>
#include <NKRenderer/Core/NkRendererConfig.h>
#include <NKRenderer/Core/NkRendererTypes.h>
#include <NKRenderer/Core/NkSceneContext.h>
#include <NKRenderer/Core/NkCamera.h>
#include <NKRenderer/Core/NkTextureLibrary.h>
#include <NKRenderer/Materials/NkMaterial.h>
#include <NKRenderer/Materials/NkMaterialSystem.h>
#include <NKRenderer/Mesh/NkMeshSystem.h>
#include <NKRenderer/Tools/Render3D/NkRender3D.h>

#include <NKFileSystem/NkPath.h>
#include <NKMath/NkMat.h>
#include <NKMath/NkVec.h>

using namespace nkentseu;
using namespace nkentseu::math;
using namespace nkentseu::renderer;

// ── Le mot du panneau ─────────────────────────────────────────────────────
static const char *kMot = "Sortie";

// ── Les couleurs ──────────────────────────────────────────────────────────
static const uint32 kCouleurMur = 0xB4AFA6FF;   // gris clair
static const uint32 kCouleurPorte = 0x52331EFF; // brun
static const uint32 kCouleurFond = 0x1B7A3CFF;  // vert de panneau de sortie
static const uint32 kCouleurTexte = 0xFFFFFFFF; // blanc

// ── L'echelle de l'image composee ─────────────────────────────────────────
// L'image du panneau est fabriquee a huit fois la taille des lettres de
// l'atlas. C'est ce qui permet de voir la difference entre une couverture
// et une distance signee.
static const float kFacteur = 8.f;

// ── La douceur du bord SDF ────────────────────────────────────────────────
// s regle la douceur du bord. On en a essaye plusieurs ; 0,05 donne le
// bord le plus net sans escalier (transition d'environ 12 pixels dans
// l'image finale, contre 2 pixels pour 0,01 : trop net, ca scintille).
static float kDouceurSDF = 0.05f;   // reglee par la variable SDF_S

// ── La porte et le panneau ────────────────────────────────────────────────
static const float kPorteX = -0.70f;
static const float kPorteH = 2.00f;
static const float kPorteZ = -2.00f;
static const float kPanneauH = 0.20f;
static const float kPanneauDecalage = 0.01f;

static void Log(const char *msg) {
	std::printf("%s\n", msg);
	std::fflush(stdout);
}

// ───────────────────────────────────────────────────────────────────────────
// La regle du nuanceur SDF du moteur (kNkFontFragSDF), recopiee.
// Le bord d'une lettre SDF est a 0,5 : une distance sous 0,5 est dehors.
// ───────────────────────────────────────────────────────────────────────────
static float smoothstep(float a, float b, float x) {
	float t = (x - a) / (b - a);
	if (t < 0.f) {
		t = 0.f;
	}
	if (t > 1.f) {
		t = 1.f;
	}
	return t * t * (3.f - 2.f * t);
}

// ───────────────────────────────────────────────────────────────────────────
// L'atlas, lu par interpolation bilineaire entre les quatre pixels voisins.
// ───────────────────────────────────────────────────────────────────────────
static float EchantillonnerAtlas(const uint8 *pixels, int largeur, int hauteur, int octets,
								  float u, float v) {
	const float fx = u * (float)largeur - 0.5f;
	const float fy = v * (float)hauteur - 0.5f;
	const int x0 = (int)std::floor(fx);
	const int y0 = (int)std::floor(fy);
	const float tx = fx - (float)x0;
	const float ty = fy - (float)y0;

	float somme = 0.f;
	for (int dy = 0; dy < 2; ++dy) {
		for (int dx = 0; dx < 2; ++dx) {
			int x = x0 + dx;
			int y = y0 + dy;
			if (x < 0) {
				x = 0;
			}
			if (y < 0) {
				y = 0;
			}
			if (x >= largeur) {
				x = largeur - 1;
			}
			if (y >= hauteur) {
				y = hauteur - 1;
			}
			const float poids = (dx ? tx : 1.f - tx) * (dy ? ty : 1.f - ty);
			somme += poids * (float)pixels[(y * largeur + x) * octets];
		}
	}
	return somme / 255.f; // 0 = rien, 1 = plein
}

// ───────────────────────────────────────────────────────────────────────────
// Fabrique l'image du panneau, a huit fois la taille des lettres.
//  - atlas ordinaire : la valeur lue divisee par 255 est la couverture ;
//  - atlas SDF        : la valeur lue divisee par 255 est la distance, et
//                       c'est le nuanceur qui en fait une couverture.
// ───────────────────────────────────────────────────────────────────────────
static bool ComposerImage(NkFontAtlas &atlas, NkFont *police, bool modeSDF, float douceur,
						   NkImage &image) {
	uint8 *pixels = nullptr;
	int largeurAtlas = 0, hauteurAtlas = 0, octets = 0;
	atlas.GetTexDataAsAlpha8(&pixels, &largeurAtlas, &hauteurAtlas, &octets);
	if (!pixels || largeurAtlas <= 0 || hauteurAtlas <= 0) {
		Log("[sdf] l'atlas n'a pas de pixels");
		return false;
	}
	if (octets <= 0) {
		octets = 1;
	}

	float largeurTotale = 0.f;
	for (const char *p = kMot; *p; ++p) {
		const NkFontGlyph *g = police->FindGlyph((NkFontCodepoint)(unsigned char)*p);
		if (g) {
			largeurTotale += g->advanceX;
		}
	}
	const int largeurImage = (int)std::lround(largeurTotale * kFacteur);
	const int hauteurImage = (int)std::lround((police->ascent - police->descent) * kFacteur);
	const int ligneBase = (int)std::lround(police->ascent * kFacteur);

	image = NkImage::Create((uint32)largeurImage, (uint32)hauteurImage, 4, kCouleurFond);
	if (!image.IsValid()) {
		Log("[sdf] creation de l'image impossible");
		return false;
	}

	float curseur = 0.f;
	for (const char *p = kMot; *p; ++p) {
		const NkFontGlyph *g = police->FindGlyph((NkFontCodepoint)(unsigned char)*p);
		if (!g) {
			curseur += police->fallbackAdvanceX;
			continue;
		}
		const int x0 = (int)std::lround((curseur + g->x0) * kFacteur);
		const int x1 = (int)std::lround((curseur + g->x1) * kFacteur);
		const int y0 = ligneBase + (int)std::lround(g->y0 * kFacteur);
		const int y1 = ligneBase + (int)std::lround(g->y1 * kFacteur);

		for (int py = y0; py < y1; ++py) {
			if (py < 0 || py >= hauteurImage) {
				continue;
			}
			const float fh = (y1 > y0) ? ((py + 0.5f - y0) / (float)(y1 - y0)) : 0.f;
			const float v = g->v0 + fh * (g->v1 - g->v0);

			for (int px = x0; px < x1; ++px) {
				if (px < 0 || px >= largeurImage) {
					continue;
				}
				const float fw = (x1 > x0) ? ((px + 0.5f - x0) / (float)(x1 - x0)) : 0.f;
				const float u = g->u0 + fw * (g->u1 - g->u0);

				const float valeur = EchantillonnerAtlas(pixels, largeurAtlas, hauteurAtlas, octets, u, v);
				float couverture = valeur;
				if (modeSDF) {
					// La valeur lue est une distance, pas une couverture :
					// c'est le nuanceur qui la transforme.
					couverture = smoothstep(0.5f - douceur, 0.5f + douceur, valeur);
				}

				math::NkColor c = image.GetPixel(px, py);
				const float inv = 1.f - couverture;
				c.r = (uint8)((float)((kCouleurTexte >> 24) & 0xFF) * couverture +
							  (float)((kCouleurFond >> 24) & 0xFF) * inv);
				c.g = (uint8)((float)((kCouleurTexte >> 16) & 0xFF) * couverture +
							  (float)((kCouleurFond >> 16) & 0xFF) * inv);
				c.b = (uint8)((float)((kCouleurTexte >> 8) & 0xFF) * couverture +
							  (float)((kCouleurFond >> 8) & 0xFF) * inv);
				c.a = 255;
				image.SetPixel(px, py, c);
			}
		}
		curseur += g->advanceX;
	}
	return true;
}

int nkmain(const NkEntryState &) {
	// La douceur du bord se regle : on en essaie plusieurs et on garde celle
	// qui donne le bord le plus net sans escalier.
	if (const char *sv = std::getenv("SDF_S")) {
		const float v = (float)std::atof(sv);
		if (v > 0.f) {
			kDouceurSDF = v;
		}
	}

	// ── Les vues ──────────────────────────────────────────────────────────
	// PANNEAU_MODE = "ordinaire" ou "sdf"
	// PANNEAU_VUE  = "3m" (trois metres) ou "50cm" (cinquante centimetres)
	const char *mode = std::getenv("PANNEAU_MODE");
	const bool modeSdf = (mode && std::strcmp(mode, "sdf") == 0);
	bool vueTroisMetres = true;
	if (const char *v = std::getenv("PANNEAU_VUE")) {
		vueTroisMetres = (std::strcmp(v, "50cm") != 0);
	}
	int shotFrame = 120;
	if (const char *sf = std::getenv("MASALLE_SHOT")) {
		shotFrame = std::atoi(sf);
		if (shotFrame <= 0) {
			shotFrame = 120;
		}
	}
	const char *shotPath = std::getenv("MASALLE_SHOT_PATH");
	if (!shotPath || !*shotPath) {
		shotPath = "capture.png";
	}
	int maxFrames = shotFrame + 6;
	if (const char *mf = std::getenv("PANNEAU_MAX_FRAMES")) {
		maxFrames = std::atoi(mf);
	}

	// ── 1. Les trois atlas ────────────────────────────────────────────────
	// (a) ordinaire a 32 pixels
	NkFontAtlas atlasOrdinaire;
	NkFont *policeOrdinaire = NkFontEmbedded::AddToAtlas(atlasOrdinaire, NkEmbeddedFontId::Inter, 32.f);
	if (!policeOrdinaire || !atlasOrdinaire.Build() || !atlasOrdinaire.IsBuilt()) {
		Log("[sdf] ERREUR : atlas ordinaire 32 impossible");
		return 1;
	}

	// (b) SDF a 32 pixels : sdfMode et sdfSpread AVANT Build()
	NkFontAtlas atlasSdf;
	atlasSdf.sdfMode = true;
	atlasSdf.sdfSpread = 6;
	NkFont *policeSdf = NkFontEmbedded::AddToAtlas(atlasSdf, NkEmbeddedFontId::Inter, 32.f);
	if (!policeSdf || !atlasSdf.Build() || !atlasSdf.IsBuilt()) {
		Log("[sdf] ERREUR : atlas SDF 32 impossible");
		return 1;
	}

	// (c) ordinaire a 128 pixels, pour mesurer l'explosion de memoire
	NkFontAtlas atlasGrand;
	NkFont *policeGrand = NkFontEmbedded::AddToAtlas(atlasGrand, NkEmbeddedFontId::Inter, 128.f);
	if (!policeGrand || !atlasGrand.Build() || !atlasGrand.IsBuilt()) {
		Log("[sdf] ERREUR : atlas ordinaire 128 impossible");
		return 1;
	}

	std::printf("[sdf] atlas ordinaire 32 : %d x %d\n", atlasOrdinaire.texWidth, atlasOrdinaire.texHeight);
	std::printf("[sdf] atlas sdf 32        : %d x %d\n", atlasSdf.texWidth, atlasSdf.texHeight);
	std::printf("[sdf] atlas ordinaire 128 : %d x %d\n", atlasGrand.texWidth, atlasGrand.texHeight);

	// atlas.txt : les trois tailles, relevees par le programme
	{
		std::FILE *f = std::fopen("atlas.txt", "w");
		if (!f) {
			Log("[sdf] ERREUR : ecriture de atlas.txt impossible");
			return 1;
		}
		std::fprintf(f, "ordinaire 32 : %d x %d\n", atlasOrdinaire.texWidth, atlasOrdinaire.texHeight);
		std::fprintf(f, "sdf 32 : %d x %d\n", atlasSdf.texWidth, atlasSdf.texHeight);
		std::fprintf(f, "ordinaire 128 : %d x %d\n", atlasGrand.texWidth, atlasGrand.texHeight);
		std::fclose(f);
	}

	// ── 2. Les deux images, de MEME taille ────────────────────────────────
	// Meme taille, sinon on mesurerait la taille et pas la methode.
	NkImage imageOrdinaire;
	if (!ComposerImage(atlasOrdinaire, policeOrdinaire, false, kDouceurSDF, imageOrdinaire)) {
		return 1;
	}
	NkImage imageSdf;
	if (!ComposerImage(atlasSdf, policeSdf, true, kDouceurSDF, imageSdf)) {
		return 1;
	}
	std::printf("[sdf] image ordinaire : %d x %d\n", (int)imageOrdinaire.Width(), (int)imageOrdinaire.Height());
	std::printf("[sdf] image sdf       : %d x %d\n", (int)imageSdf.Width(), (int)imageSdf.Height());

	// Les deux images, pour la texture.
	if (!imageOrdinaire.SavePNG("murs/panneau-ordinaire.png") ||
		!imageSdf.SavePNG("murs/panneau-sdf.png")) {
		Log("[sdf] ERREUR : ecriture des images impossible");
		return 1;
	}
	Log("[sdf] ecrit : murs/panneau-ordinaire.png et murs/panneau-sdf.png");

	if (std::getenv("SDF_COMPOSE_ONLY") != nullptr) {
		std::printf("[sdf] douceur s = %f\n", (double)kDouceurSDF);
		std::fflush(stdout);
		return 0;
	}

	const float largeurPanneau =
		kPanneauH * ((float)imageOrdinaire.Width() / (float)imageOrdinaire.Height());
	const float centrePanneauY = kPorteH + kPanneauH * 0.5f;
	const float centrePanneauZ = kPorteZ + kPanneauDecalage;

	// ── 3. La fenetre, le device, le renderer ─────────────────────────────
	NkWindowConfig wcfg;
	wcfg.title = "exo8 - le SDF, mesure";
	wcfg.width = 1280;
	wcfg.height = 720;
	wcfg.resizable = true;

	NkWindow window(wcfg);
	if (!window.IsValid()) {
		Log("[sdf] echec creation fenetre");
		return 1;
	}
	window.SetVisible(true);

	NkDeviceInitInfo di;
	di.surface = window.GetSurfaceDesc();
	di.width = wcfg.width;
	di.height = wcfg.height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(di);
	if (!device) {
		Log("[sdf] echec creation device");
		return 1;
	}

	NkRendererConfig rcfg = NkRendererConfig::ForGame(di.api, wcfg.width, wcfg.height);
	rcfg.api = di.api;
	rcfg.width = wcfg.width;
	rcfg.height = wcfg.height;
	rcfg.postProcess.ssao = false;
	rcfg.postProcess.hbao = true;

	NkRenderer *renderer = NkRenderer::Create(device, rcfg);
	if (!renderer || !renderer->Initialize() || !renderer->IsValid()) {
		Log("[sdf] echec renderer");
		return 1;
	}
	renderer->SetBackgroundColor({0.08f, 0.09f, 0.11f, 1.f});

	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshSystem *meshes = renderer->GetMeshSystem();
	NkMaterialSystem *mats = renderer->GetMaterials();
	NkTextureLibrary *texs = renderer->GetTextures();
	if (!r3d || !meshes || !mats || !texs) {
		Log("[sdf] sous-systemes manquants");
		return 1;
	}

	// ── 4. La texture du panneau ──────────────────────────────────────────
	// La face du pave affiche la texture a l'envers : on relit l'image et on
	// la retourne avant de la poser.
	NkImage imageAffichee;
	const char *fichierImage = modeSdf ? "murs/panneau-sdf.png" : "murs/panneau-ordinaire.png";
	if (!imageAffichee.Load(fichierImage)) {
		std::printf("[sdf] ERREUR : relecture de %s impossible\n", fichierImage);
		std::fflush(stdout);
		return 1;
	}
	imageAffichee.FlipHorizontal();

	NkTextureCreateDesc descPanneau;
	descPanneau.pixels = imageAffichee.Pixels();
	descPanneau.width = (uint32)imageAffichee.Width();
	descPanneau.height = (uint32)imageAffichee.Height();
	descPanneau.srgb = true;
	descPanneau.genMips = true;
	descPanneau.format = NkGPUFormat::NK_RGBA8_UNORM;
	descPanneau.debugName = "panneau";

	NkTexHandle texPanneau = texs->Create(descPanneau);
	if (!texPanneau.IsValid() || texPanneau.id == texs->GetError().id) {
		Log("[sdf] ERREUR : la texture du panneau n'a pas pu etre creee");
		return 1;
	}

	// ── 5. Les materiaux ──────────────────────────────────────────────────
	NkTexHandle texMur{};
	NkTexHandle texPorte{};
	{
		const uint32 w = 4, h = 4;
		uint8 px[4 * 4 * 4];
		for (uint32 i = 0; i < w * h; ++i) {
			px[i * 4 + 0] = (uint8)((kCouleurMur >> 24) & 0xFF);
			px[i * 4 + 1] = (uint8)((kCouleurMur >> 16) & 0xFF);
			px[i * 4 + 2] = (uint8)((kCouleurMur >> 8) & 0xFF);
			px[i * 4 + 3] = 255;
		}
		NkTextureCreateDesc d;
		d.pixels = px;
		d.width = w;
		d.height = h;
		d.srgb = true;
		d.genMips = true;
		d.format = NkGPUFormat::NK_RGBA8_UNORM;
		d.debugName = "mur";
		texMur = texs->Create(d);
	}
	{
		const uint32 w = 4, h = 4;
		uint8 px[4 * 4 * 4];
		for (uint32 i = 0; i < w * h; ++i) {
			px[i * 4 + 0] = (uint8)((kCouleurPorte >> 24) & 0xFF);
			px[i * 4 + 1] = (uint8)((kCouleurPorte >> 16) & 0xFF);
			px[i * 4 + 2] = (uint8)((kCouleurPorte >> 8) & 0xFF);
			px[i * 4 + 3] = 255;
		}
		NkTextureCreateDesc d;
		d.pixels = px;
		d.width = w;
		d.height = h;
		d.srgb = true;
		d.genMips = true;
		d.format = NkGPUFormat::NK_RGBA8_UNORM;
		d.debugName = "porte";
		texPorte = texs->Create(d);
	}

	NkMaterial *matMur = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matMur->SetAlbedo({1.f, 1.f, 1.f});
	matMur->SetRoughness(0.9f);
	matMur->SetAlbedoMap(texMur);

	NkMaterial *matPorte = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matPorte->SetAlbedo({1.f, 1.f, 1.f});
	matPorte->SetRoughness(0.6f);
	matPorte->SetAlbedoMap(texPorte);

	NkMaterial *matPanneau = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matPanneau->SetAlbedo({1.f, 1.f, 1.f});
	matPanneau->SetRoughness(0.7f);
	matPanneau->SetAlbedoMap(texPanneau);

	NkMatInstHandle instMur = matMur->GetInstHandle();
	NkMatInstHandle instPorte = matPorte->GetInstHandle();
	NkMatInstHandle instPanneau = matPanneau->GetInstHandle();

	// ── 6. La scene ───────────────────────────────────────────────────────
	NkMeshHandle cube = meshes->GetCube();
	if (!cube.IsValid()) {
		Log("[sdf] GetCube invalide");
		return 1;
	}
	const NkAABB aabbCube = meshes->GetBounds(cube);

	NkSceneContext scene;
	if (vueTroisMetres) {
		scene.camera.SetPosition({kPorteX, 1.70f, 1.00f});
	} else {
		scene.camera.SetPosition({kPorteX, 2.20f, -1.50f});
	}
	scene.camera.SetTarget({kPorteX, centrePanneauY, centrePanneauZ});
	scene.camera.SetUp({0.f, 1.f, 0.f});
	scene.camera.SetFOV(65.f);
	scene.camera.SetAspect(wcfg.width, wcfg.height);
	scene.camera.SetNearFar(0.05f, 100.f);

	NkLightDesc sun;
	sun.type = NkLightType::NK_DIRECTIONAL;
	sun.direction = {-0.3f, -1.f, -0.2f};
	sun.color = {1.f, 0.97f, 0.92f};
	sun.intensity = 0.5f;
	scene.lights.PushBack(sun);

	NkLightDesc plafonnier;
	plafonnier.type = NkLightType::NK_POINT;
	plafonnier.position = {0.f, 2.6f, 0.f};
	plafonnier.color = {1.f, 0.98f, 0.94f};
	plafonnier.intensity = 3.2f;
	plafonnier.range = 20.f;
	scene.lights.PushBack(plafonnier);

	r3d->SetIBLColor({1.f, 1.f, 1.f});
	r3d->SetIBLStrength(0.15f);

	// ── 7. La boucle ──────────────────────────────────────────────────────
	int frameCount = 0;
	while (window.IsOpen() && frameCount < maxFrames) {
		NkEvents().PollEvents();
		if (!renderer->BeginFrame()) {
			continue;
		}
		if (frameCount == shotFrame) {
			static char kShotEnv[1024];
			std::snprintf(kShotEnv, sizeof(kShotEnv), "NK_AGENT_TONELDR=%s", shotPath);
			putenv(kShotEnv);
			std::printf("[sdf] capture armee : frame %d -> %s\n", shotFrame, shotPath);
			std::fflush(stdout);
		}

		r3d->ResetFrame();
		r3d->BeginScene(scene);

		NkDrawCall3D dc;
		dc.mesh = cube;
		dc.castShadow = false;
		dc.receiveShadow = false;

		dc.material = instMur;
		dc.transform = NkMat4f::Translation({0.f, -0.1f, 0.f}) * NkMat4f::Scale({6.f, 0.2f, 4.f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		dc.transform = NkMat4f::Translation({0.f, 1.f, -2.1f}) * NkMat4f::Scale({6.f, 3.8f, 0.2f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({0.f, 1.f, 2.1f}) * NkMat4f::Scale({6.f, 3.8f, 0.2f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({-3.1f, 1.f, 0.f}) * NkMat4f::Scale({0.2f, 3.8f, 4.f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({3.1f, 1.f, 0.f}) * NkMat4f::Scale({0.2f, 3.8f, 4.f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({0.f, 3.f, 0.f}) * NkMat4f::Scale({6.f, 0.2f, 4.f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		dc.material = instPorte;
		dc.transform = NkMat4f::Translation({kPorteX, kPorteH * 0.5f, kPorteZ + 0.05f}) *
					   NkMat4f::Scale({0.8f, kPorteH, 0.1f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		dc.material = instPanneau;
		dc.transform = NkMat4f::Translation({kPorteX, centrePanneauY, centrePanneauZ}) *
					   NkMat4f::Scale({largeurPanneau, kPanneauH, 0.02f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		renderer->Present();
		renderer->EndFrame();
		++frameCount;
	}

	texs->Release(texPanneau);
	texs->Release(texMur);
	texs->Release(texPorte);
	NkMaterial::Destroy(matMur);
	NkMaterial::Destroy(matPorte);
	NkMaterial::Destroy(matPanneau);
	renderer->Shutdown();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);

	std::printf("[sdf] fin (mode : %s, vue : %s)\n", modeSdf ? "sdf" : "ordinaire",
				vueTroisMetres ? "trois metres" : "cinquante cm");
	std::fflush(stdout);
	return 0;
}
