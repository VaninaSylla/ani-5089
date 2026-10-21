// exo6 - Le chemin qui marche partout
//
// La meme salle, le meme mur, la meme texture : mais deux facons de nommer
// le fichier, et deux dossiers de lancement.
//
//   CHEMIN_MODE=relatif : Load("murs/beton.png")
//                          -> le chemin part du dossier de lancement.
//                          -> lance depuis Build/Bin/<cfg>/MaSalle/, le
//                             fichier n'est plus la : le mur reste uni.
//
//   CHEMIN_MODE=moteur  : NkPath::LocateResource("murs/beton.png")
//                          -> le moteur cherche le fichier dans plusieurs
//                             dossiers, et le trouve meme quand on est lance
//                             d'ailleurs.
//
// Fichiers rendus :
//   chemin.cpp   : ce fichier
//   essais.txt   : les quatre essais
//   reponse.txt  : la reponse redigee
//
// Contrat de frame : BeginFrame -> ResetFrame/BeginScene/Submit -> Present
// -> EndFrame. La file de drawcalls est consommee par la passe Geometry du
// render graph : il ne faut pas la flusher nous-memes.

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

#include <NKFileSystem/NkPath.h>
#include <NKMath/NkMat.h>
#include <NKMath/NkVec.h>

using namespace nkentseu;
using namespace nkentseu::math;
using namespace nkentseu::renderer;

// ── La piece ──────────────────────────────────────────────────────────────
static const float kPieceW = 8.f;
static const float kPieceH = 3.8f;
static const float kPieceD = 8.f;

// ── Le fichier de la texture, NOMME DEPUIS LE PROJET ─────────────────────
static const char *kFichierRelatif = "murs/beton.png";
static void Log(const char *msg) {
	std::printf("%s\n", msg);
	std::fflush(stdout);
}

int nkmain(const NkEntryState &) {
	// ── Quel mode de chemin ? ─────────────────────────────────────────────
	bool modeMoteur = false;
	if (const char *mode = std::getenv("CHEMIN_MODE")) {
		modeMoteur = (std::strcmp(mode, "moteur") == 0);
	}
	Log(modeMoteur ? "[chemin] mode MOTEUR : LocateResource" : "[chemin] mode RELATIF : Load");

	// ── La capture ────────────────────────────────────────────────────────
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
	if (const char *mf = std::getenv("MUR_MAX_FRAMES")) {
		maxFrames = std::atoi(mf);
	}

	// ── La fenetre, le device, le renderer ────────────────────────────────
	NkWindowConfig wcfg;
	wcfg.title = "exo6 - le chemin qui marche partout";
	wcfg.width = 1280;
	wcfg.height = 720;
	wcfg.resizable = true;

	NkWindow window(wcfg);
	if (!window.IsValid()) {
		Log("[chemin] echec creation fenetre");
		return 1;
	}
	window.SetVisible(true);

	NkDeviceInitInfo di;
	di.surface = window.GetSurfaceDesc();
	di.width = wcfg.width;
	di.height = wcfg.height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(di);
	if (!device) {
		Log("[chemin] echec creation device");
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
		Log("[chemin] echec renderer");
		return 1;
	}
	renderer->SetBackgroundColor({0.08f, 0.09f, 0.11f, 1.f});

	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshSystem *meshes = renderer->GetMeshSystem();
	NkMaterialSystem *mats = renderer->GetMaterials();
	NkTextureLibrary *texs = renderer->GetTextures();
	if (!r3d || !meshes || !mats || !texs) {
		Log("[chemin] sous-systemes manquants");
		return 1;
	}

	// ── LE CHEMIN DE LA TEXTURE ──────────────────────────────────────────
	// C'est tout l'objet de l'exercice. Le reste du programme est identique
	// dans les quatre essais.
	//
	// Le nom du fichier peut etre change par la variable d'environnement
	// CHEMIN_FICHIER : c'est ce qui permet de faire le essai "le fichier
	// n'existe pas" (le moteur rend alors sa texture d'erreur, magenta).
	const char *fichier = kFichierRelatif;
	bool fichierRemplace = false;
	if (const char *f = std::getenv("CHEMIN_FICHIER")) {
		if (*f) {
			fichier = f;
			fichierRemplace = true;
		}
	}
	std::printf("[chemin] dossier de lancement : %s\n", NkPath::GetCurrentDirectory().CStr());

	NkTexHandle texMur{};
	NkString cheminUtilise;

	if (modeMoteur) {
		// Le chemin du moteur : il cherche dans plusieurs dossiers. S'il ne
		// trouve rien, il rend une chaine VIDE : il faut le tester avant de
		// le donner a Load.
		cheminUtilise = NkPath::LocateResource("murs/beton.png", true);
		if (fichierRemplace) {
			// Variante "le fichier n'existe pas" : c'est la seule chose que
			// l'essai change, la maniere de nommer le fichier reste celle
			// du moteur.
			cheminUtilise = NkPath::LocateResource(fichier, true);
		}
		if (cheminUtilise.Empty()) {
			Log("[chemin] LocateResource n'a rien trouve : chaine vide, on ne charge rien");
		} else {
			Log(cheminUtilise.CStr());
			texMur = texs->Load(cheminUtilise.CStr());
		}
	} else {
		// Le chemin relatif : il part du dossier de lancement, et de rien
		// d'autre.
		cheminUtilise = fichier;
		Log(fichier);
		texMur = texs->Load(fichier);
	}

	// ATTENTION : quand le chargement echoue, le moteur NE rend pas un
	// handle nul : il rend un handle valide qui pointe sa texture d'erreur
	// (le magenta 1x1). Le handle seul ne dit donc rien ; il faut le
	// comparer a la texture d'erreur, et lire le journal du moteur.
	const bool repli = (texMur.id == texs->GetError().id);
	if (texMur.IsValid() && !repli) {
		Log("[chemin] texture chargee : le mur sera texture");
	} else if (repli) {
		Log("[chemin] ECHEC : le moteur a rendu sa texture d'erreur (magenta), "
			"le mur sera magenta, pas uni");
	} else {
		Log("[chemin] ECHEC : aucune texture, le mur restera uni");
	}

	// ── La geometrie et les materiaux ────────────────────────────────────
	NkMeshHandle cube = meshes->GetCube();
	if (!cube.IsValid()) {
		Log("[chemin] GetCube invalide");
		return 1;
	}
	const NkAABB aabbCube = meshes->GetBounds(cube);

	NkMaterial *matUni = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matUni->SetAlbedo({0.72f, 0.70f, 0.66f});
	matUni->SetRoughness(0.90f);
	matUni->SetMetallic(0.0f);

	NkMaterial *matText = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matText->SetAlbedo({1.f, 1.f, 1.f});
	matText->SetRoughness(0.90f);
	matText->SetMetallic(0.0f);
	if (texMur.IsValid()) {
		matText->SetAlbedoMap(texMur);
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

	NkLightDesc plafonnier;
	plafonnier.type = NkLightType::NK_POINT;
	plafonnier.position = {0.f, 2.6f, 0.f};
	plafonnier.color = {1.f, 0.98f, 0.94f};
	plafonnier.intensity = 3.2f;
	plafonnier.range = 20.f;
	scene.lights.PushBack(plafonnier);

	r3d->SetIBLColor({1.f, 1.f, 1.f});
	r3d->SetIBLStrength(0.15f);

	// ── La boucle ─────────────────────────────────────────────────────────
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
			std::printf("[chemin] capture armee : frame %d -> %s\n", shotFrame, shotPath);
			std::fflush(stdout);
		}

		r3d->ResetFrame();
		r3d->BeginScene(scene);

		NkDrawCall3D dc;
		dc.mesh = cube;
		dc.castShadow = false;
		dc.receiveShadow = false;

		// Le sol.
		dc.material = instUni;
		dc.transform = NkMat4f::Translation({0.f, -1.f, 0.f}) * NkMat4f::Scale({kPieceW, 0.2f, kPieceD});
		// Le moteur elague les draws avec dc.aabb TEL QUEL, comme une boite
		// EN MONDE : il ne lui applique pas la transformee. Il faut donc lui
		// donner les bornes reelles du pave, apres transformee.
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		// Le mur du fond : le SEUL pave qui recoit le materiau texture.
		dc.material = instText;
		dc.transform = NkMat4f::Translation({0.f, 1.f, -4.f}) * NkMat4f::Scale({kPieceW, kPieceH, 0.2f});
		// Le moteur elague les draws avec dc.aabb TEL QUEL, comme une boite
		// EN MONDE : il ne lui applique pas la transformee. Il faut donc lui
		// donner les bornes reelles du pave, apres transformee.
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		// Les trois autres murs, et le plafond : ils restent unis.
		dc.material = instUni;
		dc.transform = NkMat4f::Translation({-4.f, 1.f, 0.f}) * NkMat4f::Scale({0.2f, kPieceH, kPieceD});
		// Le moteur elague les draws avec dc.aabb TEL QUEL, comme une boite
		// EN MONDE : il ne lui applique pas la transformee. Il faut donc lui
		// donner les bornes reelles du pave, apres transformee.
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({4.f, 1.f, 0.f}) * NkMat4f::Scale({0.2f, kPieceH, kPieceD});
		// Le moteur elague les draws avec dc.aabb TEL QUEL, comme une boite
		// EN MONDE : il ne lui applique pas la transformee. Il faut donc lui
		// donner les bornes reelles du pave, apres transformee.
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({0.f, 1.f, 4.f}) * NkMat4f::Scale({kPieceW, kPieceH, 0.2f});
		// Le moteur elague les draws avec dc.aabb TEL QUEL, comme une boite
		// EN MONDE : il ne lui applique pas la transformee. Il faut donc lui
		// donner les bornes reelles du pave, apres transformee.
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({0.f, 3.f, 0.f}) * NkMat4f::Scale({kPieceW, 0.2f, kPieceD});
		// Le moteur elague les draws avec dc.aabb TEL QUEL, comme une boite
		// EN MONDE : il ne lui applique pas la transformee. Il faut donc lui
		// donner les bornes reelles du pave, apres transformee.
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		renderer->Present();
		renderer->EndFrame();
		++frameCount;
	}

	// On ne libere que la texture que NOUS avons chargee : la texture
	// d'erreur du moteur lui appartient, on ne la touche pas.
	if (texMur.IsValid() && texMur.id != texs->GetError().id) {
		texs->Release(texMur);
	}
	NkMaterial::Destroy(matUni);
	NkMaterial::Destroy(matText);
	renderer->Shutdown();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);

	std::printf("[chemin] fin\n");
	std::fflush(stdout);
	return 0;
}
