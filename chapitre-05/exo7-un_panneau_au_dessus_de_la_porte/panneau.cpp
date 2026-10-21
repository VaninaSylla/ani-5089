// exo7 - Un panneau au-dessus de la porte
//
// On ecrit le mot "Sortie" avec une police EMBARQUEE dans le moteur, on
// compose l'image lettre par lettre en lisant l'atlas, et on pose cette
// image sur un pave de 20 cm de haut, au-dessus de la porte de la salle.
//
// Fichiers rendus :
//   panneau.cpp          : ce fichier
//   panneau.png          : l'image du mot, composee depuis l'atlas
//   trois-metres.png     : le panneau vu de trois metres
//   cinquante-cm.png     : le panneau vu de cinquante centimetres
//   panneau.txt          : les tailles et les trois paragraphes
//
// La police ne vient d'aucun fichier : elle est dans le binaire du moteur
// (NkFontEmbedded). L'atlas est construit par le programme, et chaque
// lettre est lue dans l'atlas par ses coordonnees (u0, v0, u1, v1), jamais
// recopiee a la main.
//
// Contrat de frame : BeginFrame -> ResetFrame/BeginScene/Submit -> Present
// -> EndFrame. La file de drawcalls est consommee par la passe Geometry du
// render graph : il ne faut pas la flusher nous-memes.

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
// La couleur d'albedo d'un materiau SANS carte n'est pas prise en compte :
// le moteur lie alors le blanc 1x1 sur le canal albedo ("REPLI BLANC") et
// l'objet sort gris. Pour avoir une couleur, il faut donc une carte.
// On fabrique ici des cartes d'une seule couleur, en memoire.
static const uint32 kCouleurMur = 0xB4AFA6FF;   // gris clair
static const uint32 kCouleurPorte = 0x52331EFF; // brun
static const uint32 kCouleurFond = 0x1B7A3CFF;  // vert de panneau de sortie
static const uint32 kCouleurTexte = 0xFFFFFFFF; // blanc

// ── La porte et le panneau ────────────────────────────────────────────────
static const float kPorteX = -0.70f;   // la porte est centree en x = -0,70 m
static const float kPorteH = 2.00f;    // haute de 2 m
static const float kPorteZ = -2.00f;   // contre le mur du fond, en z = -2 m
static const float kPanneauH = 0.20f;  // le pave du panneau fait 20 cm de haut
static const float kPanneauDecalage = 0.01f; // 1 cm devant le mur (sinon ca scintille)

static void Log(const char *msg) {
	std::printf("%s\n", msg);
	std::fflush(stdout);
}

// ───────────────────────────────────────────────────────────────────────────
// Fabrique une texture d'une seule couleur, en memoire (4 x 4 pixels RGBA).
// ───────────────────────────────────────────────────────────────────────────
static NkTexHandle TextureUnie(NkTextureLibrary *texs, uint32 couleur, const char *nom) {
	const uint32 w = 4, h = 4;
	uint8 pixels[4 * 4 * 4];
	for (uint32 i = 0; i < w * h; ++i) {
		pixels[i * 4 + 0] = (uint8)((couleur >> 24) & 0xFF);
		pixels[i * 4 + 1] = (uint8)((couleur >> 16) & 0xFF);
		pixels[i * 4 + 2] = (uint8)((couleur >> 8) & 0xFF);
		pixels[i * 4 + 3] = (uint8)(couleur & 0xFF);
	}
	NkTextureCreateDesc desc;
	desc.pixels = pixels;
	desc.width = w;
	desc.height = h;
	desc.srgb = true;
	desc.genMips = true;
	desc.format = NkGPUFormat::NK_RGBA8_UNORM;
	desc.debugName = nom;
	return texs->Create(desc);
}

// ───────────────────────────────────────────────────────────────────────────
// Compose l'image du mot : on part d'un fond uni, et pour chaque lettre on
// lit la couverture dans l'atlas, pixel par pixel, en passant par les
// coordonnees u0, v0, u1, v1 du glyphe.
// ───────────────────────────────────────────────────────────────────────────
static bool ComposerLeMot(NkFontAtlas &atlas, NkFont *police, NkImage &image) {
	// Les pixels de l'atlas : un octet par pixel, qui dit combien la lettre
	// le couvre.
	uint8 *pixels = nullptr;
	int32 largeurAtlas = 0, hauteurAtlas = 0, octetsParPixel = 0;
	atlas.GetTexDataAsAlpha8(&pixels, &largeurAtlas, &hauteurAtlas, &octetsParPixel);
	if (!pixels || largeurAtlas <= 0 || hauteurAtlas <= 0) {
		Log("[panneau] l'atlas n'a pas de pixels");
		return false;
	}
	if (octetsParPixel <= 0) {
		octetsParPixel = 1;
	}
	const int32 strideAtlas = largeurAtlas * octetsParPixel;

	// La largeur de l'image est la somme des avanceX des six lettres ; la
	// hauteur vaut ascent - descent, et la ligne de base est a ascent pixels
	// du haut.
	float largeurTotale = 0.f;
	for (const char *p = kMot; *p; ++p) {
		const NkFontGlyph *g = police->FindGlyph((NkFontCodepoint)(unsigned char)*p);
		if (g) {
			largeurTotale += g->advanceX;
		}
	}
	const int32 largeurImage = (int32)std::lround(largeurTotale);
	const int32 hauteurImage = (int32)std::lround(police->ascent - police->descent);
	const int32 ligneBase = (int32)std::lround(police->ascent);

	image = NkImage::Create((uint32)largeurImage, (uint32)hauteurImage, 4, kCouleurFond);
	if (!image.IsValid()) {
		Log("[panneau] creation de l'image impossible");
		return false;
	}

	// On compose lettre par lettre.
	float curseur = 0.f;
	for (const char *p = kMot; *p; ++p) {
		const char lettre = *p;
		const NkFontGlyph *g = police->FindGlyph((NkFontCodepoint)(unsigned char)lettre);
		if (!g) {
			Log("[panneau] glyphe introuvable");
			curseur += police->fallbackAdvanceX;
			continue;
		}

		// Le rectangle de la lettre dans l'image : de curseur + x0 a
		// curseur + x1, et de base + y0 a base + y1 (y0 est negatif pour ce
		// qui monte au-dessus de la ligne de base).
		const int32 x0 = (int32)std::lround(curseur + g->x0);
		const int32 x1 = (int32)std::lround(curseur + g->x1);
		const int32 y0 = ligneBase + (int32)std::lround(g->y0);
		const int32 y1 = ligneBase + (int32)std::lround(g->y1);

		for (int32 py = y0; py < y1; ++py) {
			if (py < 0 || py >= hauteurImage) {
				continue;
			}
			// Position verticale, en fraction du rectangle du glyphe.
			const float fh = (y1 > y0) ? ((py + 0.5f - y0) / (float)(y1 - y0)) : 0.f;
			const float v = g->v0 + fh * (g->v1 - g->v0);
			int32 ay = (int32)(v * (float)hauteurAtlas);
			if (ay < 0) {
				ay = 0;
			}
			if (ay >= hauteurAtlas) {
				ay = hauteurAtlas - 1;
			}

			for (int32 px = x0; px < x1; ++px) {
				if (px < 0 || px >= largeurImage) {
					continue;
				}
				// Position horizontale, en fraction du rectangle du glyphe.
				const float fw = (x1 > x0) ? ((px + 0.5f - x0) / (float)(x1 - x0)) : 0.f;
				const float u = g->u0 + fw * (g->u1 - g->u0);
				int32 ax = (int32)(u * (float)largeurAtlas);
				if (ax < 0) {
					ax = 0;
				}
				if (ax >= largeurAtlas) {
					ax = largeurAtlas - 1;
				}

				// La couverture du glyphe a cet endroit, de 0 a 1.
				const uint8 couverture = pixels[ay * strideAtlas + ax * octetsParPixel];
				const float alpha = (float)couverture / 255.f;

				// On melange la couleur du texte avec celle du fond, dans
				// cette proportion.
				math::NkColor c = image.GetPixel(px, py);
				const float inv = 1.f - alpha;
				c.r = (uint8)((float)((kCouleurTexte >> 24) & 0xFF) * alpha +
							  (float)((kCouleurFond >> 24) & 0xFF) * inv);
				c.g = (uint8)((float)((kCouleurTexte >> 16) & 0xFF) * alpha +
							  (float)((kCouleurFond >> 16) & 0xFF) * inv);
				c.b = (uint8)((float)((kCouleurTexte >> 8) & 0xFF) * alpha +
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
	// ── La vue demandee ───────────────────────────────────────────────────
	// PANNEAU_VUE = "3m" (trois metres) ou "50cm" (cinquante centimetres).
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

	// ── 1. L'atlas, avec la police embarquee du moteur ───────────────────
	// Aucun fichier de police : la police est dans le binaire.
	NkFontAtlas atlas;
	NkFont *police = NkFontEmbedded::AddToAtlas(atlas, NkEmbeddedFontId::Inter, 32.f);
	if (!police) {
		Log("[panneau] ERREUR : la police embarquee n'a pas pu etre ajoutee a l'atlas");
		return 1;
	}
	if (!atlas.Build()) {
		Log("[panneau] ERREUR : l'atlas n'a pas pu etre construit");
		return 1;
	}
	if (!atlas.IsBuilt() || atlas.texWidth <= 0 || atlas.texHeight <= 0) {
		Log("[panneau] ERREUR : l'atlas construit est vide");
		return 1;
	}
	std::printf("[panneau] atlas : %d x %d\n", atlas.texWidth, atlas.texHeight);
	std::printf("[panneau] police : ascent %f, descent %f\n", (double)police->ascent,
				(double)police->descent);

	// ── 2. L'image du mot ─────────────────────────────────────────────────
	NkImage image;
	if (!ComposerLeMot(atlas, police, image)) {
		return 1;
	}
	std::printf("[panneau] image : %d x %d\n", (int)image.Width(), (int)image.Height());
	if (!image.SavePNG("panneau.png")) {
		Log("[panneau] ERREUR : ecriture de panneau.png impossible");
		return 1;
	}
	Log("[panneau] ecrit : panneau.png");

	// Le pave du panneau garde les proportions de l'image.
	const float largeurPanneau = kPanneauH * ((float)image.Width() / (float)image.Height());
	const float centrePanneauY = kPorteH + kPanneauH * 0.5f; // juste au-dessus de la porte
	const float centrePanneauZ = kPorteZ + kPanneauDecalage;

	// ── 3. La fenetre, le device, le renderer ─────────────────────────────
	NkWindowConfig wcfg;
	wcfg.title = "exo7 - un panneau au-dessus de la porte";
	wcfg.width = 1280;
	wcfg.height = 720;
	wcfg.resizable = true;

	NkWindow window(wcfg);
	if (!window.IsValid()) {
		Log("[panneau] echec creation fenetre");
		return 1;
	}
	window.SetVisible(true);

	NkDeviceInitInfo di;
	di.surface = window.GetSurfaceDesc();
	di.width = wcfg.width;
	di.height = wcfg.height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(di);
	if (!device) {
		Log("[panneau] echec creation device");
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
		Log("[panneau] echec renderer");
		return 1;
	}
	renderer->SetBackgroundColor({0.08f, 0.09f, 0.11f, 1.f});

	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshSystem *meshes = renderer->GetMeshSystem();
	NkMaterialSystem *mats = renderer->GetMaterials();
	NkTextureLibrary *texs = renderer->GetTextures();
	if (!r3d || !meshes || !mats || !texs) {
		Log("[panneau] sous-systemes manquants");
		return 1;
	}

	// ── 4. La texture du panneau ──────────────────────────────────────────
	// On relit le PNG que l'on vient d'ecrire, et on le retourne
	// horizontalement : la face du pave qui regarde la piece affiche la
	// texture a l'envers (le mot sorrait "eitroS"). On retourne donc
	// l'image, et le PNG reste, lui, dans le bon sens.
	NkImage imageAffichee;
	if (!imageAffichee.Load("panneau.png")) {
		Log("[panneau] ERREUR : relecture de panneau.png impossible");
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
	if (texPanneau.IsValid() && texPanneau.id != texs->GetError().id) {
		Log("[panneau] texture du panneau prete");
	} else {
		Log("[panneau] ERREUR : la texture du panneau n'a pas pu etre creee");
		return 1;
	}

	// ── 5. Les materiaux ──────────────────────────────────────────────────
	// Le mur et la porte ont eux aussi besoin d'une carte, sinon le moteur
	// lie le blanc 1x1 et ignore la couleur d'albedo.
	NkTexHandle texMur = TextureUnie(texs, kCouleurMur, "mur");
	NkTexHandle texPorte = TextureUnie(texs, kCouleurPorte, "porte");

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
		Log("[panneau] GetCube invalide");
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
			std::printf("[panneau] capture armee : frame %d -> %s\n", shotFrame, shotPath);
			std::fflush(stdout);
		}

		r3d->ResetFrame();
		r3d->BeginScene(scene);

		// ATTENTION : le moteur elague les draws avec dc.aabb TEL QUEL, comme
		// une boite EN MONDE : il ne lui applique pas la transformee. Il faut
		// donc lui donner les bornes reelles du pave, apres transformee.
		NkDrawCall3D dc;
		dc.mesh = cube;
		dc.castShadow = false;
		dc.receiveShadow = false;

		// Le sol.
		dc.material = instMur;
		dc.transform = NkMat4f::Translation({0.f, -0.1f, 0.f}) * NkMat4f::Scale({6.f, 0.2f, 4.f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		// Le mur du fond, a z = -2 : c'est lui qui porte la porte et le panneau.
		dc.transform = NkMat4f::Translation({0.f, 1.f, -2.1f}) * NkMat4f::Scale({6.f, 3.8f, 0.2f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		// Les trois autres murs et le plafond.
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

		// La porte : centree en x = -0,70, haute de 2 m, contre le mur du fond.
		dc.material = instPorte;
		dc.transform = NkMat4f::Translation({kPorteX, kPorteH * 0.5f, kPorteZ + 0.05f}) *
					   NkMat4f::Scale({0.8f, kPorteH, 0.1f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		// Le panneau : le pave qui porte l'image du mot, au-dessus de la porte.
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

	std::printf("[panneau] fin (vue : %s)\n", vueTroisMetres ? "trois metres" : "cinquante cm");
	std::fflush(stdout);
	return 0;
}
