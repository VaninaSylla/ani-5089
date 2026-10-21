// exo4 - Le premier mur texture
//
// On prend une salle en cubes, et on habille UN SEUL de ses murs avec une
// photo de mur. Deux captures sont faites du meme point de vue : une avant
// que le mur ne soit texture (avant.png), une apres (apres.png).
//
// Fichiers rendus :
//   mur.cpp      : ce fichier
//   avant.png    : le mur du fond, sans texture
//   apres.png    : le meme mur, avec la texture
//   journal.txt  : la ligne du moteur qui annonce la texture et ses dimensions
//   reponse.txt  : la reponse redigee
//
// Une salle de reference (chapitre 4) est reprise ici : fenetre, device
// Vulkan, renderer, sol + 4 murs + plafond.
//
// Contrat de frame : BeginFrame -> ResetFrame/BeginScene/Submit -> Present
// -> EndFrame. La file de drawcalls est consommee par la passe Geometry du
// render graph : il ne faut surtout PAS flusher nous-memes la file, sinon elle
// est videe avant la passe et rien n'est dessine.

#include <cstdio>
#include <cstdlib>
#include <cstring>

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

#include <NKMath/NkMat.h>
#include <NKMath/NkVec.h>

using namespace nkentseu;
using namespace nkentseu::math;
using namespace nkentseu::renderer;

// ── La piece ──────────────────────────────────────────────────────────────
static const float kPieceW = 8.f;   // largeur  (x)
static const float kPieceH = 3.8f;  // hauteur des murs : de -0,9 (le sol) a 2,9 (le plafond)
static const float kPieceD = 8.f;   // profondeur (z)

// ── Le chemin de la texture, relatif, comme le demande l'exercice ────────
// RELATIF : il part du dossier d'ou le programme est lance. C'est le sujet
// de l'exercice suivant ("le chemin qui marche partout") ; ici on veut voir
// le mur texte, pas la panne.
static const char *kCheminTexture = "murs/beton.png";

// ───────────────────────────────────────────────────────────────────────────
// Un pave de la salle : un cube, un materiau, une place, une taille.
// Le materiau est un parametre : c'est ce qui permet d'avoir un pave
// texture et des paves unis, avec le meme code.
// ───────────────────────────────────────────────────────────────────────────
static NkDrawCall3D Pave(NkMeshHandle cube, const NkAABB &aabb, NkMatInstHandle mat,
						 const NkVec3f &pos, const NkVec3f &scale) {
	NkDrawCall3D dc;
	dc.mesh = cube;
	dc.material = mat;
	dc.transform = NkMat4f::Translation(pos) * NkMat4f::Scale(scale);
	// Le moteur elague les draws avec dc.aabb TEL QUEL, comme une boite EN
	// MONDE : il ne lui applique pas la transformee. Il faut donc lui donner
	// les bornes reelles du pave, apres transformee.
	dc.aabb = aabb.Transformed(dc.transform);
	// La piece est fermee : le soleil n'y entre pas, et une carte d'ombres
	// dans une piece fermee ne donne que des stries. On n'en fait donc pas.
	dc.castShadow = false;
	dc.receiveShadow = false;
	return dc;
}

// ───────────────────────────────────────────────────────────────────────────
// La variante demandee par l'exercice : le pave du mur du fond, qui recoit
// le materiau texture. UN SEUL mur l'emploie ; les autres restent unis.
// (Elle prend le materiau en parametre comme Pave, mais elle est dediee au
//  mur texture : c'est elle qu'on voit changer entre avant.png et apres.png.)
// ───────────────────────────────────────────────────────────────────────────
static NkDrawCall3D PaveMurTexture(NkMeshHandle cube, const NkAABB &aabb, NkMatInstHandle matText,
								   const NkVec3f &pos, const NkVec3f &scale) {
	return Pave(cube, aabb, matText, pos, scale);
}

static void Log(const char *msg) {
	std::printf("%s\n", msg);
	std::fflush(stdout);
}

int nkmain(const NkEntryState &) {
	// ── L'etat de la capture ──────────────────────────────────────────────
	// MASALLE_SHOT      : numero de la frame a capturer (120 par defaut)
	// MASALLE_SHOT_PATH : nom du fichier .png ecrit
	// MUR_SANS_TEXTURE  : a 1, le mur du fond garde son materiau uni.
	//                     C'est l'etat "avant" ; sans cette variable, le mur
	//                     est texture : c'est l'etat "apres".
	//
	// Le moteur possede une sonde de relecture de l'image finale (le
	// transient ToneLDR), pilotee par NK_AGENT_TONELDR=<png>. On arme la
	// sonde au moment de la frame demandee : la capture montre donc
	// l'image de CETTE frame. La sonde ne tire qu'une fois par processus :
	// la salle se lance donc deux fois, une fois pour avant.png, une fois
	// pour apres.png, avec le meme point de vue.
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
	const bool sansTexture = (std::getenv("MUR_SANS_TEXTURE") != nullptr) &&
							 (std::strcmp(std::getenv("MUR_SANS_TEXTURE"), "0") != 0);

	// On sort peu de temps apres la capture, pour que le programme se ferme
	// tout seul (et pour que la capture soit prise, elle ne l'est qu'une fois).
	int maxFrames = shotFrame + 6;
	if (const char *mf = std::getenv("MUR_MAX_FRAMES")) {
		maxFrames = std::atoi(mf);
	}

	// ── La fenetre ────────────────────────────────────────────────────────
	NkWindowConfig wcfg;
	wcfg.title = "exo4 - le premier mur texture";
	wcfg.width = 1280;
	wcfg.height = 720;
	wcfg.resizable = true;

	NkWindow window(wcfg);
	if (!window.IsValid()) {
		Log("[mur] echec creation fenetre");
		return 1;
	}
	window.SetVisible(true);

	// ── Le device ─────────────────────────────────────────────────────────
	NkDeviceInitInfo di;
	di.surface = window.GetSurfaceDesc();
	di.width = wcfg.width;
	di.height = wcfg.height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(di);
	if (!device) {
		Log("[mur] echec creation device");
		return 1;
	}

	// ── Le renderer ───────────────────────────────────────────────────────
	NkRendererConfig rcfg = NkRendererConfig::ForGame(di.api, wcfg.width, wcfg.height);
	rcfg.api = di.api;
	rcfg.width = wcfg.width;
	rcfg.height = wcfg.height;
	// Le SSAO du moteur strie les surfaces vues de biais (les murs lateraux,
	// le sol). On lui prend HBAO, qui rend les memes coins sans les stries :
	// la comparaison avant/apres porte sur la texture, pas sur le bruit.
	rcfg.postProcess.ssao = false;
	rcfg.postProcess.hbao = true;

	NkRenderer *renderer = NkRenderer::Create(device, rcfg);
	if (!renderer || !renderer->Initialize() || !renderer->IsValid()) {
		Log("[mur] echec renderer");
		return 1;
	}
	renderer->SetBackgroundColor({0.08f, 0.09f, 0.11f, 1.f});

	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshSystem *meshes = renderer->GetMeshSystem();
	NkMaterialSystem *mats = renderer->GetMaterials();
	NkTextureLibrary *texs = renderer->GetTextures();
	if (!r3d || !meshes || !mats || !texs) {
		Log("[mur] sous-systemes manquants");
		return 1;
	}

	// ── La geometrie ──────────────────────────────────────────────────────
	NkMeshHandle cube = meshes->GetCube();
	if (!cube.IsValid()) {
		Log("[mur] GetCube invalide");
		return 1;
	}
	const NkAABB aabbCube = meshes->GetBounds(cube);

	// ── La texture : chargee UNE FOIS, avant la boucle ───────────────────
	// Le moteur annonce dans son journal (logs/app.log) chaque texture
	// chargee, avec ses dimensions. C'est cette ligne qu'on recopie dans
	// journal.txt. Pour la revoir, lancer avec NK_TEX_CACHE=0.
	NkTexHandle texMur = texs->Load(kCheminTexture);
	if (!texMur.IsValid()) {
		Log("[mur] ATTENTION : texture introuvable ou illisible (le mur restera uni)");
	}

	// ── Les materiaux ─────────────────────────────────────────────────────
	// Materiau uni : sert au sol, au plafond et aux trois autres murs.
	NkMaterial *matUni = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matUni->SetAlbedo({0.72f, 0.70f, 0.66f});
	matUni->SetRoughness(0.90f);
	matUni->SetMetallic(0.0f);

	// Materiau du mur texture : la photo est donnee en carte d'albedo.
	NkMaterial *matText = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matText->SetAlbedo({1.f, 1.f, 1.f});
	matText->SetRoughness(0.90f);
	matText->SetMetallic(0.0f);
	if (texMur.IsValid()) {
		matText->SetAlbedoMap(texMur);
		// La texture se repete tous les metres, au lieu d'etre etiree sur
		// les 8 metres du mur.
		matText->SetTriplanarTileSize(1.0f);
	}

	NkMatInstHandle instUni = matUni->GetInstHandle();
	NkMatInstHandle instText = matText->GetInstHandle();

	// ── La scene ──────────────────────────────────────────────────────────
	NkSceneContext scene;
	scene.camera.SetPosition({0.f, 1.4f, 3.2f});
	scene.camera.SetTarget({0.f, 1.2f, 0.f});
	scene.camera.SetUp({0.f, 1.f, 0.f});
	scene.camera.SetFOV(65.f);
	scene.camera.SetAspect(wcfg.width, wcfg.height);
	scene.camera.SetNearFar(0.05f, 200.f);

	NkLightDesc sun;
	sun.type = NkLightType::NK_DIRECTIONAL;
	sun.direction = {-0.4f, -1.f, -0.3f};
	sun.color = {1.f, 0.97f, 0.92f};
	sun.intensity = 0.6f;
	scene.lights.PushBack(sun);

	// Une lampe au plafond de la piece : la piece est fermee, le soleil seul
	// n'eclaire rien dedans.
	NkLightDesc plafonnier;
	plafonnier.type = NkLightType::NK_POINT;
	plafonnier.position = {0.f, 2.6f, 0.f};
	plafonnier.color = {1.f, 0.98f, 0.94f};
	plafonnier.intensity = 3.2f;
	plafonnier.range = 20.f;
	scene.lights.PushBack(plafonnier);

	// L'ambiance. Les champs ambientIntensity/ambientColor du contexte ne
	// sont jamais lus par le moteur : c'est SetIBLStrength/SetIBLColor qui
	// agissent reellement.
	r3d->SetIBLColor({1.f, 1.f, 1.f});
	r3d->SetIBLStrength(0.15f);

	// ── La boucle ─────────────────────────────────────────────────────────
	int frameCount = 0;
	while (window.IsOpen() && frameCount < maxFrames) {
		NkEvents().PollEvents();
		if (!renderer->BeginFrame()) {
			continue;
		}

		// La capture est armee a la frame demandee : la sonde du moteur tire
		// alors sur l'image de cette frame.
		if (frameCount == shotFrame) {
			static char kShotEnv[1024];
			std::snprintf(kShotEnv, sizeof(kShotEnv), "NK_AGENT_TONELDR=%s", shotPath);
			putenv(kShotEnv);
			std::printf("[mur] capture armee : frame %d -> %s\n", shotFrame, shotPath);
			std::fflush(stdout);
		}

		r3d->ResetFrame();
		r3d->BeginScene(scene);

		// Le sol.
		r3d->Submit(Pave(cube, aabbCube, instUni, {0.f, -1.f, 0.f}, {kPieceW, 0.2f, kPieceD}));

		// Le mur du fond : le SEUL pave qui recoit le materiau texture.
		if (sansTexture) {
			r3d->Submit(Pave(cube, aabbCube, instUni, {0.f, 1.f, -4.f}, {kPieceW, kPieceH, 0.2f}));
		} else {
			r3d->Submit(PaveMurTexture(cube, aabbCube, instText, {0.f, 1.f, -4.f},
									   {kPieceW, kPieceH, 0.2f}));
		}

		// Les trois autres murs, et le plafond : ils restent unis.
		r3d->Submit(Pave(cube, aabbCube, instUni, {-4.f, 1.f, 0.f}, {0.2f, kPieceH, kPieceD}));
		r3d->Submit(Pave(cube, aabbCube, instUni, {4.f, 1.f, 0.f}, {0.2f, kPieceH, kPieceD}));
		r3d->Submit(Pave(cube, aabbCube, instUni, {0.f, 1.f, 4.f}, {kPieceW, kPieceH, 0.2f}));
		r3d->Submit(Pave(cube, aabbCube, instUni, {0.f, 3.f, 0.f}, {kPieceW, 0.2f, kPieceD}));

		renderer->Present();
		renderer->EndFrame();
		++frameCount;
	}

	// ── La fermeture ──────────────────────────────────────────────────────
	if (texMur.IsValid()) {
		texs->Release(texMur);
	}
	NkMaterial::Destroy(matUni);
	NkMaterial::Destroy(matText);
	renderer->Shutdown();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);

	std::printf("[mur] fin (etat : %s)\n", sansTexture ? "AVANT, mur uni" : "APRES, mur texture");
	std::fflush(stdout);
	return 0;
}
