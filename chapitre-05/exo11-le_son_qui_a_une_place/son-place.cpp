// exo11 - Le son qui a une place
//
// Une salle avec une fenetre, et de la pluie qui tombe juste derriere la
// fenetre. Le son est place dans l'espace, en boucle, et l'auditeur suit la
// camera : on se place donc a trois endroits de la salle pour ecouter.
//
// Fichiers rendus :
//   son-place.cpp : ce fichier (la salle et le son)
//   mesure.cpp     : ce fichier (les mesures, sans fenetre et sans son)
//   mesures.txt    : les six mesures
//   ecoute.txt     : les cinq paragraphes
//
// Le son est fabrique par le moteur (AudioGenerator::GenerateNoise, bruit
// rose de 4 secondes) et joue en boucle. Il est place AVANT le Play, dans
// p.source3d : apres, les reglages ne se changent plus que par les
// fonctions SetSource du moteur.
//
// L'auditeur, lui, est pose une fois, au demarrage, a la place de la
// camera : c'est lui qui change quand on change de point de vue.
//
// Contrat de frame : BeginFrame -> ResetFrame/BeginScene/Submit -> Present
// -> EndFrame. Et dc.aabb est donne EN MONDE, apres transformation : le
// moteur elague les draws avec cette boite telle quelle.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

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

// L'audio vient APRES la fenetre : NKAudio et NKTime ouvrent un espace de
// noms "time" qui fait echouer le time(nullptr) de NkRandom.h (tire par
// NKWindow) si celui-ci est lu apres.
#include <NKAudio/NkAudio.h>
#include <NKTime/NkChrono.h>

using namespace nkentseu;
using namespace nkentseu::math;
using namespace nkentseu::renderer;
using namespace nkentseu::audio;

// ── La fenetre et la pluie ────────────────────────────────────────────────
// La fenetre est dans le mur de gauche (x = -2). La pluie tombe juste
// derriere elle.
static const float kPluiePos[3] = {-1.98f, 1.40f, -0.90f};
static const float kPluieMin = 1.0f;   // en deca de cette distance, le son ne grossit plus
static const float kPluieMax = 20.0f;  // au-dela, il se tait

// ── Les trois points d'ecoute ─────────────────────────────────────────────
// 1 : pres de la fenetre, 2 : au centre, 3 : dans le coin oppose.
struct Ecoute {
	const char *nom;
	float pos[3];
	float cible[3];
};
static const Ecoute kEcoutes[3] = {
	{"pres de la fenetre", {-1.55f, 1.60f, -0.90f}, {-2.00f, 1.40f, -0.90f}},
	{"au centre", {0.00f, 1.60f, 0.00f}, {-1.98f, 1.40f, -0.90f}},
	{"coin oppose", {1.60f, 1.60f, 1.60f}, {-1.98f, 1.40f, -0.90f}},
};

static void Log(const char *msg) {
	std::printf("%s\n", msg);
	std::fflush(stdout);
}

int nkmain(const NkEntryState &) {
	// ── Le point d'ecoute demande ─────────────────────────────────────────
	int numEcoute = 1;
	if (const char *e = std::getenv("SALLE_ECOUTE")) {
		numEcoute = std::atoi(e);
		if (numEcoute < 1 || numEcoute > 3) {
			numEcoute = 1;
		}
	}
	// SALLE_SANS_POSITIONAL=1 : le son n'est plus place, il sort au milieu.
	const bool sansPositional = (std::getenv("SALLE_SANS_POSITIONAL") != nullptr);
	const Ecoute &ecoute = kEcoutes[numEcoute - 1];

	int maxFrames = 300; // environ 5 secondes a 60 images par seconde
	if (const char *mf = std::getenv("SALLE_MAX_FRAMES")) {
		maxFrames = std::atoi(mf);
	}

	// ── 1. Le moteur de son ───────────────────────────────────────────────
	AudioEngineConfig acfg;
	acfg.sampleRate = 48000;
	acfg.enableHrtf = true; // rendu binaural : c'est ce qu'on entend au casque
	AudioEngine &moteur = AudioEngine::Instance();
	if (!moteur.Initialize(acfg)) {
		Log("[son] echec : moteur de son");
		return 1;
	}
	// Le jeu de filtres binauraux : genere par le moteur, aucun fichier a
	// charger. C'est la version synthetique (modele spherique), moins precise
	// qu'un jeu mesure en laboratoire, mais immediate et sans dependance.
	if (!moteur.GenerateSyntheticHrtf()) {
		Log("[son] ATTENTION : le jeu HRTF n'a pas pu etre genere");
	}

	// ── 2. La pluie, en boucle, placee dans l'espace ─────────────────────
	// Le son est fabrique par le moteur : aucun fichier a livrer.
	AudioSample pluie = AudioGenerator::GenerateNoise(4.f, WaveformType::NOISE_PINK);

	VoiceParams p;
	p.looping = true;
	p.source3d.positional = !sansPositional;
	p.source3d.position[0] = kPluiePos[0];
	if (const char *sx = std::getenv("SALLE_SOURCE_X")) {
		p.source3d.position[0] = (float)std::atof(sx);
		std::printf("[son] source decalee en x = %f\n", (double)p.source3d.position[0]);
	}
	p.source3d.position[1] = kPluiePos[1];
	p.source3d.position[2] = kPluiePos[2];
	p.source3d.minDistance = kPluieMin;
	p.source3d.maxDistance = kPluieMax;
	p.source3d.rolloffFactor = 1.0f;
	p.source3d.attenuationModel = AttenuationModel::INVERSE;
	p.source3d.useHrtf = true; // cette voix est spatialisee en binaural
	AudioHandle voix = moteur.Play(pluie, p);
	(void)voix;

	// ── 3. L'auditeur, a la place de la camera ───────────────────────────
	// Une fois, au demarrage. Le regard va vers la fenetre, le haut est le
	// haut.
	const float fx = ecoute.cible[0] - ecoute.pos[0];
	const float fy = ecoute.cible[1] - ecoute.pos[1];
	const float fz = ecoute.cible[2] - ecoute.pos[2];
	const float inv = 1.f / std::sqrt(fx * fx + fy * fy + fz * fz);
	moteur.SetListenerPosition(ecoute.pos[0], ecoute.pos[1], ecoute.pos[2]);
	moteur.SetListenerOrientation(fx * inv, fy * inv, fz * inv, 0.f, 1.f, 0.f);
	Log(sansPositional ? "[son] ecoute SANS positional" : "[son] ecoute AVEC positional");

	// ── 4. La fenetre, le device, le renderer ────────────────────────────
	NkWindowConfig wcfg;
	wcfg.title = "exo11 - le son qui a une place";
	wcfg.width = 1280;
	wcfg.height = 720;
	wcfg.resizable = true;

	NkWindow window(wcfg);
	if (!window.IsValid()) {
		Log("[son] echec fenetre");
		moteur.Shutdown();
		AudioLoader::Free(pluie);
		return 1;
	}
	window.SetVisible(true);

	NkDeviceInitInfo di;
	di.surface = window.GetSurfaceDesc();
	di.width = wcfg.width;
	di.height = wcfg.height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(di);
	if (!device) {
		Log("[son] echec device");
		moteur.Shutdown();
		AudioLoader::Free(pluie);
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
		Log("[son] echec renderer");
		moteur.Shutdown();
		AudioLoader::Free(pluie);
		return 1;
	}
	renderer->SetBackgroundColor({0.08f, 0.09f, 0.11f, 1.f});

	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshSystem *meshes = renderer->GetMeshSystem();
	NkMaterialSystem *mats = renderer->GetMaterials();
	if (!r3d || !meshes || !mats) {
		Log("[son] sous-systemes manquants");
		moteur.Shutdown();
		AudioLoader::Free(pluie);
		return 1;
	}

	// ── 5. La salle ───────────────────────────────────────────────────────
	// Piece de 4 x 3 x 4 metres, avec une fenetre dans le mur de gauche
	// (x = -2), a hauteur d'yeux.
	const uint32 kCouleurMur = 0xB4AFA6FF;
	const uint32 kCouleurSol = 0x8C8C8CFF;
	NkTextureLibrary *texs = renderer->GetTextures();

	auto TextureUnie = [texs](uint32 couleur, const char *nom) {
		uint8 px[4 * 4 * 4];
		for (int i = 0; i < 16; ++i) {
			px[i * 4 + 0] = (uint8)((couleur >> 24) & 0xFF);
			px[i * 4 + 1] = (uint8)((couleur >> 16) & 0xFF);
			px[i * 4 + 2] = (uint8)((couleur >> 8) & 0xFF);
			px[i * 4 + 3] = 255;
		}
		NkTextureCreateDesc d;
		d.pixels = px;
		d.width = 4;
		d.height = 4;
		d.srgb = true;
		d.genMips = true;
		d.format = NkGPUFormat::NK_RGBA8_UNORM;
		d.debugName = nom;
		return texs->Create(d);
	};

	NkTexHandle texMur = TextureUnie(kCouleurMur, "mur");
	NkTexHandle texSol = TextureUnie(kCouleurSol, "sol");

	NkMaterial *matMur = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matMur->SetAlbedo({1.f, 1.f, 1.f});
	matMur->SetRoughness(0.9f);
	matMur->SetAlbedoMap(texMur);

	NkMaterial *matSol = NkMaterial::Create(mats, NkMaterialType::NK_PBR_METALLIC);
	matSol->SetAlbedo({1.f, 1.f, 1.f});
	matSol->SetRoughness(0.8f);
	matSol->SetAlbedoMap(texSol);

	NkMatInstHandle instMur = matMur->GetInstHandle();
	NkMatInstHandle instSol = matSol->GetInstHandle();

	NkMeshHandle cube = meshes->GetCube();
	const NkAABB aabbCube = meshes->GetBounds(cube);

	NkSceneContext scene;
	scene.camera.SetPosition({ecoute.pos[0], ecoute.pos[1], ecoute.pos[2]});
	scene.camera.SetTarget({ecoute.cible[0], ecoute.cible[1], ecoute.cible[2]});
	scene.camera.SetUp({0.f, 1.f, 0.f});
	scene.camera.SetFOV(65.f);
	scene.camera.SetAspect(wcfg.width, wcfg.height);
	scene.camera.SetNearFar(0.05f, 100.f);

	NkLightDesc plafonnier;
	plafonnier.type = NkLightType::NK_POINT;
	plafonnier.position = {0.f, 2.6f, 0.f};
	plafonnier.color = {1.f, 0.98f, 0.94f};
	plafonnier.intensity = 3.2f;
	plafonnier.range = 20.f;
	scene.lights.PushBack(plafonnier);

	r3d->SetIBLColor({1.f, 1.f, 1.f});
	r3d->SetIBLStrength(0.15f);

	// ── 6. La boucle ──────────────────────────────────────────────────────
	int frameCount = 0;
	while (window.IsOpen() && frameCount < maxFrames) {
		NkEvents().PollEvents();
		if (!renderer->BeginFrame()) {
			continue;
		}
		r3d->ResetFrame();
		r3d->BeginScene(scene);

		NkDrawCall3D dc;
		dc.mesh = cube;
		dc.castShadow = false;
		dc.receiveShadow = false;

		// Le sol et le plafond.
		dc.material = instSol;
		dc.transform = NkMat4f::Translation({0.f, -0.1f, 0.f}) * NkMat4f::Scale({4.f, 0.2f, 4.f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.material = instMur;
		dc.transform = NkMat4f::Translation({0.f, 3.f, 0.f}) * NkMat4f::Scale({4.f, 0.2f, 4.f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		// Les trois murs sans fenetre.
		dc.transform = NkMat4f::Translation({0.f, 1.f, -2.1f}) * NkMat4f::Scale({4.f, 3.8f, 0.2f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({0.f, 1.f, 2.1f}) * NkMat4f::Scale({4.f, 3.8f, 0.2f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({2.1f, 1.f, 0.f}) * NkMat4f::Scale({0.2f, 3.8f, 4.f});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		// Le mur de gauche (x = -2), percE d'une fenetre : on le fait en
		// quatre morceaux autour de l'ouverture.
		const float fenetreZ = kPluiePos[2];
		const float fenetreY = kPluiePos[1];
		const float demiLargeur = 0.6f;  // la fenetre fait 1,2 m de large
		const float demiHauteur = 0.5f;  // et 1 m de haut
		// Sous la fenetre.
		dc.transform = NkMat4f::Translation({-2.1f, (fenetreY - demiHauteur) * 0.5f, fenetreZ}) *
					   NkMat4f::Scale({0.2f, fenetreY - demiHauteur, 2.f * demiLargeur});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		// Au-dessus de la fenetre.
		dc.transform = NkMat4f::Translation({-2.1f, fenetreY + demiHauteur * 0.5f, fenetreZ}) *
					   NkMat4f::Scale({0.2f, 3.f - (fenetreY + demiHauteur), 2.f * demiLargeur});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		// A gauche et a droite de la fenetre.
		const float deMarge = 2.f - 2.f * demiLargeur; // 0,8 m de mur de part et d'autre
		dc.transform = NkMat4f::Translation({-2.1f, fenetreY, -1.f - deMarge * 0.5f}) *
					   NkMat4f::Scale({0.2f, 2.f * demiHauteur, deMarge});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);
		dc.transform = NkMat4f::Translation({-2.1f, fenetreY, 1.f + deMarge * 0.5f}) *
					   NkMat4f::Scale({0.2f, 2.f * demiHauteur, deMarge});
		dc.aabb = aabbCube.Transformed(dc.transform);
		r3d->Submit(dc);

		renderer->Present();
		renderer->EndFrame();
		++frameCount;
	}

	// ── 7. La fermeture ───────────────────────────────────────────────────
	// La voix est arretee avant le moteur, puis le moteur est ferme, et
	// l'echantillon est libere en DERNIER : une voix qui joue encore lit ses
	// donnees.
	moteur.Stop(voix);
	moteur.Shutdown();
	AudioLoader::Free(pluie);

	texs->Release(texMur);
	texs->Release(texSol);
	NkMaterial::Destroy(matMur);
	NkMaterial::Destroy(matSol);
	renderer->Shutdown();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);

	Log("[son] fin");
	return 0;
}
