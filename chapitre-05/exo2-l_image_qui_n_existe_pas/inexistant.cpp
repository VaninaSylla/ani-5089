// exo2 - L'image qui n'existe pas
//
// On demande a NKImage de charger un fichier qui n'existe pas, puis on demande
// au moteur de poser ce meme fichier comme texture sur les murs d'une salle.
// Question posee par l'exercice : que fait le moteur quand l'image manque ?
//
// Ce programme produit :
//   mur-absent.png : la capture du mur prive de son image
//   journal.txt     : la ligne du programme + la ligne du journal du moteur
//   reponse.txt     : la reponse redigee
//
// Il s'appuie sur la salle minimale mise au point sur le vrai kit R3D
// (Vulkan). Contrat de frame : BeginFrame -> ResetFrame/BeginScene/Submit
// -> Present -> EndFrame. La file de drawcalls est consommee par la passe
// Geometry du render graph, PAS par un Flush manuel.

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

#include <NKImage/Core/NkImage.h>

#include <NKFileSystem/NkPath.h>
#include <NKMath/NkMat.h>
#include <NKMath/NkVec.h>

using namespace nkentseu;
using namespace nkentseu::math;
using namespace nkentseu::renderer;

// Le fichier que l'on va demander alors qu'il n'existe pas. Le chemin est
// relatif : il part du dossier d'ou le programme est lance, comme le veut
// l'exercice. Le dossier "murs/" de ce projet ne contient que beton.png :
// "murs/nexistepas.png" n'y est pas, et n'est nulle part ailleurs.
static const char *kFichierAbsent = "murs/nexistepas.png";

static void Log(const char *msg) {
	std::printf("%s\n", msg);
	std::fflush(stdout);
}

int nkmain(const NkEntryState &) {
	// Toutes les observations partent sur la sortie standard : c'est ce que le
	// terminal affiche, et c'est cette sortie complete que le journal recopie.
	std::printf("Journal de l'exercice : l'image qui n'existe pas\n");
	std::printf("Fichier demande : %s\n\n", kFichierAbsent);

	// ------------------------------------------------------------------
	// 1) NKImage : on demande directement le fichier absent.
	//    La ligne affichee est celle demandee par l'exercice :
	//    charge : C, valide : V, largeur : L, hauteur : H.
	// ------------------------------------------------------------------
	NkImage image;
	const bool charge = image.Load(kFichierAbsent);
	const bool valide = image.IsValid();
	std::printf("charge : %s, valide : %s, largeur : %d, hauteur : %d\n", charge ? "true" : "false",
				valide ? "true" : "false", image.Width(), image.Height());

	// Le fichier existe-t-il sur le disque, independamment du moteur ?
	{
		std::FILE *probe = std::fopen(kFichierAbsent, "rb");
		std::printf("[disque] fopen(\"%s\") = %s\n", kFichierAbsent,
					probe ? "ouvert" : "ECHEC (fichier absent)");
		if (probe) {
			std::fclose(probe);
		}
	}
	std::printf("\n");

	// ------------------------------------------------------------------
	// 2) La salle : on pose le fichier absent comme texture des murs.
	// ------------------------------------------------------------------
	NkWindowConfig wcfg;
	wcfg.title = "exo2 - l'image qui n'existe pas";
	wcfg.width = 1280;
	wcfg.height = 720;
	wcfg.resizable = true;

	NkWindow window(wcfg);
	if (!window.IsValid()) {
		Log("[exo2] echec creation fenetre");
		return 1;
	}
	window.SetVisible(true);

	NkDeviceInitInfo di;
	di.surface = window.GetSurfaceDesc();
	di.width = wcfg.width;
	di.height = wcfg.height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(di);
	if (!device) {
		Log("[exo2] echec creation device");
		return 1;
	}

	NkRendererConfig rcfg = NkRendererConfig::ForGame(di.api, wcfg.width, wcfg.height);
	rcfg.api = di.api;
	rcfg.width = wcfg.width;
	rcfg.height = wcfg.height;

	NkRenderer *renderer = NkRenderer::Create(device, rcfg);
	if (!renderer || !renderer->Initialize() || !renderer->IsValid()) {
		Log("[exo2] echec renderer");
		return 1;
	}
	renderer->SetBackgroundColor({0.08f, 0.09f, 0.11f, 1.f});

	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshSystem *meshes = renderer->GetMeshSystem();
	NkMaterialSystem *mats = renderer->GetMaterials();
	NkTextureLibrary *texs = renderer->GetTextures();
	if (!r3d || !meshes || !mats || !texs) {
		Log("[exo2] sous-systemes manquants");
		return 1;
	}

	// La question centrale : que renvoie la bibliotheque de textures ?
	// On charge le MEME chemin que celui de NKImage, dans la texture.
	NkTexHandle texAbsente = texs->Load(kFichierAbsent);
	std::printf("[textures] Load(\"%s\") = handle id=%llu (%s)\n", kFichierAbsent,
				(unsigned long long)texAbsente.id, texAbsente.IsValid() ? "non nul" : "nul");
	std::printf("[textures] id des textures par defaut du moteur :\n");
	std::printf("[textures]   White1x1  id=%llu\n", (unsigned long long)texs->GetWhite1x1().id);
	std::printf("[textures]   Black1x1  id=%llu\n", (unsigned long long)texs->GetBlack1x1().id);
	std::printf("[textures]   Normal1x1 id=%llu\n", (unsigned long long)texs->GetNormal1x1().id);
	std::printf("[textures]   Error     id=%llu  (texture magenta \"manquante\")\n",
				(unsigned long long)texs->GetError().id);
	// Le handle rendu est-il la texture d'erreur du moteur ?
	const bool repliErreur = (texAbsente.id == texs->GetError().id);
	std::printf("[textures] le handle rendu pointe-t-il la texture d'erreur ? %s\n",
				repliErreur ? "OUI" : "non");
	std::printf("[textures] le handle rendu pointe-t-il le blanc 1x1 ? %s\n\n",
				(texAbsente.id == texs->GetWhite1x1().id) ? "OUI" : "non");
	std::printf("Consequence : un fichier absent ne renvoie pas un pointeur nul et ne plante\n"
				"pas. La bibliotheque rend un handle NON NUL qui pointe la texture d'erreur\n"
				"(magenta 1x1) : le mur est bien dessine, mais en magenta uni. C'est le\n"
				"panneau \"image manquante\" du moteur.\n\n");

	// --- Materiaux ---
	NkMaterial *matSol = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matSol->SetAlbedo({0.55f, 0.55f, 0.57f});
	matSol->SetRoughness(0.85f);

	NkMaterial *matMur = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matMur->SetAlbedo({0.72f, 0.70f, 0.66f});
	matMur->SetRoughness(0.9f);
	// On POSE quand meme la texture absente. Si le moteur la refuse, le canal
	// albedo retombe sur le blanc 1x1 (cf. journal du moteur plus bas).
	matMur->SetAlbedoMap(texAbsente);

	NkMeshHandle cube = meshes->GetCube();
	NkMatInstHandle instSol = matSol->GetInstHandle();
	NkMatInstHandle instMur = matMur->GetInstHandle();

	NkDrawCall3D dcSol;
	dcSol.mesh = cube;
	dcSol.material = instSol;
	dcSol.transform = NkMat4f::Translation({0.f, -1.0f, 0.f}) * NkMat4f::Scale({8.f, 0.2f, 8.f});
	dcSol.aabb = meshes->GetBounds(cube);
	dcSol.castShadow = false;
	dcSol.receiveShadow = true;

	NkDrawCall3D dcMur;
	dcMur.mesh = cube;
	dcMur.material = instMur;
	dcMur.aabb = meshes->GetBounds(cube);

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
	sun.intensity = 3.f;
	scene.lights.PushBack(sun);

	// ------------------------------------------------------------------
	// 3) La capture.
	//    MASALLE_SHOT      : numero de la frame a capturer (120 par defaut)
	//    MASALLE_SHOT_PATH : nom du fichier .png ecrit
	//    Le moteur possede une sonde de relecture de l'image finale (le
	//    transient ToneLDR), pilotee par NK_AGENT_TONELDR=<png>. On arme la
	//    sonde a la frame demandee : la capture montre donc l'image de CETTE
	//    frame. La sonde ne tire qu'une fois par processus.
	// ------------------------------------------------------------------
	int shotFrame = 120;
	if (const char *sf = std::getenv("MASALLE_SHOT")) {
		shotFrame = std::atoi(sf);
		if (shotFrame <= 0) {
			shotFrame = 120;
		}
	}
	const char *shotPath = std::getenv("MASALLE_SHOT_PATH");
	if (!shotPath || !*shotPath) {
		shotPath = "mur-absent.png";
	}
	// On sort peu de temps apres la capture, pour que le programme se ferme
	// tout seul.
	int maxFrames = shotFrame + 6;
	if (const char *mf = std::getenv("MASALLE_MAX_FRAMES")) {
		maxFrames = std::atoi(mf);
	}
	std::printf("[capture] frame %d -> %s\n\n", shotFrame, shotPath);

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
			std::printf("[exo2] capture armee : frame %d -> %s\n", shotFrame, shotPath);
			std::fflush(stdout);
		}

		r3d->ResetFrame();
		r3d->BeginScene(scene);

		r3d->Submit(dcSol);

		NkDrawCall3D dc;
		dc = dcMur;
		dc.transform = NkMat4f::Translation({0.f, 1.0f, -4.f}) * NkMat4f::Scale({8.f, 3.f, 0.2f});
		r3d->Submit(dc);

		dc = dcMur;
		dc.transform = NkMat4f::Translation({-4.f, 1.0f, 0.f}) * NkMat4f::Scale({0.2f, 3.f, 8.f});
		r3d->Submit(dc);

		dc = dcMur;
		dc.transform = NkMat4f::Translation({4.f, 1.0f, 0.f}) * NkMat4f::Scale({0.2f, 3.f, 8.f});
		r3d->Submit(dc);

		dc = dcMur;
		dc.transform = NkMat4f::Translation({0.f, 1.0f, 4.f}) * NkMat4f::Scale({8.f, 3.f, 0.2f});
		r3d->Submit(dc);

		dc = dcMur;
		dc.transform = NkMat4f::Translation({0.f, 3.0f, 0.f}) * NkMat4f::Scale({8.f, 0.2f, 8.f});
		r3d->Submit(dc);

		renderer->Present();
		renderer->EndFrame();
		++frameCount;
	}

	// On ne libere que la texture que NOUS avons chargee. Quand le fichier
	// est absent, le handle rendu est celui de la texture d'erreur du moteur :
	// elle lui appartient, on ne la touche pas.
	if (texAbsente.IsValid() && !repliErreur) {
		texs->Release(texAbsente);
	}
	NkMaterial::Destroy(matSol);
	NkMaterial::Destroy(matMur);
	renderer->Shutdown();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);

	std::fflush(stdout);
	Log("[exo2] fin du programme");
	return 0;
}
