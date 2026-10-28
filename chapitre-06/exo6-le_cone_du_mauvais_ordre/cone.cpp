#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKEvent/NkEventSystem.h"
#include "NKEvent/NkWindowEvent.h"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKEvent/NkMouseEvent.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkAngle.h"

#include "NKRHI/Core/NkDeviceFactory.h"
#include "NKRenderer/NkRenderer.h"
#include "NKRenderer/Core/NkCamera.h"
#include "NKRenderer/Core/NkRenderGraph.h"
#include "NKRenderer/Core/NkTextureLibrary.h"
#include "NKRenderer/Materials/NkMaterialCollection.h"
#include "NKRenderer/Mesh/NkMeshSystem.h"
#include "NKRenderer/Tools/Offscreen/NkOffscreenTarget.h"
#include "NKRenderer/Tools/Render2D/NkRender2D.h"
#include "NKRenderer/Tools/Render3D/NkRender3D.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace nkentseu;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) { d.appName = "MaSalle"; }
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {

	const float32 kPiece = 4.00f;
	const float32 kPlafond = 2.50f;
	const float32 kMur = 0.10f;
	const float32 kPorteL = 0.90f;
	const float32 kPorteH = 2.00f;
	const float32 kFenL = 1.20f;
	const float32 kFenH = 1.00f;
	const float32 kFenY = 1.40f;
	const float32 kTableL = 1.20f;
	const float32 kTableP = 0.80f;
	const float32 kTableH = 0.75f;
	const float32 kYeux = 1.70f;

	const float32 kDemi = kPiece * 0.5f;

	const float32 kRadVersDeg = 57.2957795f;
	const float32 kTangageFixe = 30.f;	 // degrés, figé : c'est le cône qu'on
									 // promène par le lacet
	const float32 kSensibilite = 0.002f; // radians par comptage de la souris

	uint64 EnvU64(const char *nom, uint64 defaut) {
		const char *t = getenv(nom);
		if (t == nullptr || *t == '\0') return defaut;
		return (uint64)strtoull(t, nullptr, 10);
	}

	// L'angle, en degrés, au-dessus de l'horizon : arcsinus de la hauteur y
	// du regard. C'est la « hauteur » qu'on imprime et qu'on recopie.
	float32 HauteurDeg(const NkVec3f &regard) {
		float32 y = regard.y;
		if (y > 1.f) y = 1.f;
		if (y < -1.f) y = -1.f;
		return std::asinf(y) * kRadVersDeg;
	}

	// Le regard par la voie quaternion, telle que la caméra le calcule.
	// ordreBon : q = qLacet * qTangage (le tangage s'effectue d'abord) ;
	// sinon    : q = qTangage * qLacet (le tangage tourne autour de l'axe
	// du MONDE, après le lacet) — l'ordre fautif, celui d'exo5.
	NkVec3f RegardQuat(float32 lacetDeg, bool ordreBon) {
		const NkQuatf qLacet = NkQuatf::RotateY(NkAngle(lacetDeg));
		const NkQuatf qTangage = NkQuatf::RotateX(NkAngle(kTangageFixe));
		const NkQuatf q = ordreBon ? (qLacet * qTangage) : (qTangage * qLacet);
		return q * NkVec3f{ 0.f, 0.f, -1.f };
	}

	// Au démarrage : les dix hauteurs 
	void LancerLesHauteurs() {
		const float32 lacets[] = { 0.f, 45.f, 90.f, 135.f, 180.f };
		for (int ordre = 0; ordre < 2; ++ordre) {
			const bool ordreBon = (ordre == 0);
			const char *nom = ordreBon ? "lacet*tangage" : "tangage*lacet";
			for (float32 lacetDeg : lacets) {
				const float32 hauteur = HauteurDeg(RegardQuat(lacetDeg, ordreBon));
				std::printf("%s, lacet %.0f : hauteur %.2f deg\n",
							nom, (double)lacetDeg, (double)hauteur);
			}
		}
		std::fflush(stdout);
	}

	void Pave(NkRender3D *r3d, NkMeshSystem *meshes, const NkVec3f &centre,
			  const NkVec3f &taille, const NkVec3f &teinte, float32 rugosite = 1.f) {
		NkDrawCall3D dc;
		dc.mesh = meshes->GetCube();
		dc.transform = NkMat4f::Translate(centre) * NkMat4f::Scale(taille);
		dc.aabb = { { centre.x - taille.x * 0.5f, centre.y - taille.y * 0.5f,
					  centre.z - taille.z * 0.5f },
					{ centre.x + taille.x * 0.5f, centre.y + taille.y * 0.5f,
					  centre.z + taille.z * 0.5f } };
		dc.tint = teinte;
		dc.metallic = 0.f;
		dc.roughness = rugosite;
		r3d->Submit(dc);
	}

	void SoumettreLaSalle(NkRender3D *r3d, NkMeshSystem *meshes) {
		const NkVec3f grisMur = { 0.72f, 0.70f, 0.66f };
		const NkVec3f grisSol = { 0.38f, 0.36f, 0.34f };
		const NkVec3f bois = { 0.55f, 0.38f, 0.22f };
		const NkVec3f vitre = { 0.75f, 0.85f, 0.92f };

		{
			NkDrawCall3D dc;
			dc.mesh = meshes->GetPlane();
			dc.transform = NkMat4f::Scale({ kPiece, 1.f, kPiece });
			dc.aabb = { { -kDemi, 0.f, -kDemi }, { kDemi, 0.f, kDemi } };
			dc.castShadow = false;
			dc.tint = grisSol;
			dc.metallic = 0.f;
			dc.roughness = 1.f;
			r3d->Submit(dc);
		}

		const float32 yMur = kPlafond * 0.5f;
		const float32 bord = kDemi + kMur * 0.5f;

		Pave(r3d, meshes, { 0.f, yMur, -bord }, { kPiece, kPlafond, kMur }, grisMur);
		Pave(r3d, meshes, { 0.f, yMur, bord }, { kPiece, kPlafond, kMur }, grisMur);
		Pave(r3d, meshes, { -bord, yMur, 0.f }, { kMur, kPlafond, kPiece }, grisMur);
		Pave(r3d, meshes, { bord, yMur, 0.f }, { kMur, kPlafond, kPiece }, grisMur);

		Pave(r3d, meshes, { 0.f, kPlafond + kMur * 0.5f, 0.f },
			 { kPiece, kMur, kPiece }, grisMur);

		Pave(r3d, meshes, { -0.70f, kPorteH * 0.5f, -kDemi + 0.02f },
			 { kPorteL, kPorteH, 0.04f }, bois, 0.6f);

		Pave(r3d, meshes, { -kDemi + 0.02f, kFenY, -0.90f },
			 { 0.04f, kFenH, kFenL }, vitre, 0.15f);
		Pave(r3d, meshes, { -kDemi + 0.02f, kFenY, 0.90f },
			 { 0.04f, kFenH, kFenL }, vitre, 0.15f);

		const float32 tx = 0.60f, tz = 0.70f;
		const float32 ep = 0.05f;
		Pave(r3d, meshes, { tx, kTableH - ep * 0.5f, tz },
			 { kTableL, ep, kTableP }, bois, 0.5f);

		const float32 pied = 0.06f;
		const float32 hp = kTableH - ep;
		for (int i = 0; i < 4; ++i) {
			const float32 dx = ((i & 1) ? 1.f : -1.f) * (kTableL * 0.5f - pied);
			const float32 dz = ((i & 2) ? 1.f : -1.f) * (kTableP * 0.5f - pied);
			Pave(r3d, meshes, { tx + dx, hp * 0.5f, tz + dz },
				 { pied, hp, pied }, bois, 0.6f);
		}

		Pave(r3d, meshes, { -1.10f, 0.50f, 1.20f }, { 1.f, 1.f, 1.f },
			 { 0.85f, 0.80f, 0.30f }, 0.8f);
	}

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	//  Avant la fenêtre : les dix hauteurs (hauteurs.txt) 
	LancerLesHauteurs();

	NkWindowConfig cfg;
	cfg.title = "Ma salle";
	cfg.width = 1280;
	cfg.height = 720;
	cfg.centered = true;
	cfg.vsync = true;

	NkWindow fenetre(cfg);
	if (!fenetre.IsValid()) {
		std::printf("[MaSalle] la fenetre n'a pas pu s'ouvrir\n");
		return 1;
	}

	NkDeviceInitInfo devInit{};
	devInit.surface = fenetre.GetSurfaceDesc();
	devInit.width = fenetre.GetSize().x;
	devInit.height = fenetre.GetSize().y;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInit);
	if (device == nullptr) {
		std::printf("[MaSalle] aucune API graphique disponible\n");
		return 1;
	}

	NkRendererConfig cfgEcran =
		NkRendererConfig::For2D(devInit.api, devInit.width, devInit.height);
	NkRenderer *rEcran = NkRenderer::Create(device, cfgEcran);
	if (rEcran == nullptr || !rEcran->Initialize()) {
		std::printf("[MaSalle] le renderer d'ecran n'a pas pu s'initialiser\n");
		NkDeviceFactory::Destroy(device);
		return 1;
	}

	NkOffscreenDesc od;
	od.width = devInit.width;
	od.height = devInit.height;
	od.hasDepth = true;
	od.hdr = false;
	od.colorFmt = NkGPUFormat::NK_RGBA8_UNORM;
	od.readable = true;
	od.readback = true;
	od.name = "MaSalle_Cible";

	NkOffscreenTarget *cible = rEcran->CreateOffscreen(od);
	if (cible == nullptr) {
		std::printf("[MaSalle] cible hors ecran refusee\n");
		NkRenderer::Destroy(rEcran);
		NkDeviceFactory::Destroy(device);
		return 1;
	}

	NkRendererConfig cfgSalle =
		NkRendererConfig::ForGame(devInit.api, devInit.width, devInit.height);
	cfgSalle.Disable(NK_SS_RENDER2D);
	cfgSalle.Disable(NK_SS_TEXT);
	cfgSalle.Disable(NK_SS_OVERLAY);
	cfgSalle.Enable(NK_SS_OFFSCREEN);
	if (EnvU64("MASALLE_POST", 1) == 0) {
		cfgSalle.Disable(NK_SS_POST_PROCESS);
	}

	NkRenderer *renderer = NkRenderer::Create(device, cfgSalle);
	if (renderer == nullptr || !renderer->Initialize()) {
		std::printf("[MaSalle] le renderer de la salle n'a pas pu s'initialiser\n");
		NkRenderer::Destroy(rEcran);
		NkDeviceFactory::Destroy(device);
		return 1;
	}
	renderer->SetFinalColorTarget(
		rEcran->GetTextures()->GetRHIHandle(cible->GetColorResult()));

	// Pose figée par l'environnement : lacet en degrés, ordre au départ.
	const bool lacetFigee = getenv("MASALLE_LACET") != nullptr;
	const float32 lacetFigeeDeg = (float32)atof(getenv("MASALLE_LACET") != nullptr
												 ? getenv("MASALLE_LACET") : "0");
	bool ordreBon = (EnvU64("MASALLE_ORDRE", 0) == 0);
	float32 lacetDeg = lacetFigee ? lacetFigeeDeg : 0.f;

	std::printf("[cone] tangage fixe a %.0f deg ; Espace bascule l'ordre ; "
				"lacet fige a %.0f deg (souris %s)\n",
				(double)kTangageFixe, (double)lacetDeg,
				lacetFigee ? "inactive" : "active");
	std::printf("[cone] ordre : %s\n", ordreBon ? "lacet*tangage" : "tangage*lacet");
	std::fflush(stdout);

	const uint64 imageDeSortie = EnvU64("MASALLE_EXIT", 0);
	const uint64 imageDeCapture = EnvU64("MASALLE_SHOT", 0);

	const NkVec3f position = { 0.30f, kYeux, 1.20f };

	uint64 image = 0;
	bool tourne = true;

	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr) {
				tourne = false;
			}
			if (NkKeyPressEvent *kp = ev->As<NkKeyPressEvent>()) {
				if (kp->GetKey() == NkKey::NK_SPACE) {
					ordreBon = !ordreBon;
					std::printf("[cone] ordre : %s\n",
								ordreBon ? "lacet*tangage" : "tangage*lacet");
					std::fflush(stdout);
				}
			}
			// La souris : chaque comptage est compté UNE FOIS, à son arrivée
			// dans la pompe — c'est le drainage d'exo3, appliqué au lacet.
			if (!lacetFigee) {
				if (NkMouseRawEvent *ms = ev->As<NkMouseRawEvent>()) {
					lacetDeg += (float32)ms->GetDeltaX() * kSensibilite *
								kRadVersDeg;
				}
			}
		}
		if (!tourne) break;

		if (rEcran->BeginFrame()) {
			NkICommandBuffer *cmd = rEcran->GetCmd();

			NkRender3D *r3d = renderer->GetRender3D();

			renderer->FlushGraphRebuilds();
			r3d->ResetFrame();
			if (auto *mc = renderer->GetMaterialCollection()) {
				mc->Upload();
			}

			// La caméra, par quaternion : REGARD et HAUT du même q — l'up
			// suit l'ordre lui-même, c'est ce qui fait pencher l'horizon
			// avec l'ordre fautif (mauvais-90.png).
			const NkQuatf qLacet = NkQuatf::RotateY(NkAngle(lacetDeg));
			const NkQuatf qTangage = NkQuatf::RotateX(NkAngle(kTangageFixe));
			const NkQuatf q = ordreBon ? (qLacet * qTangage) : (qTangage * qLacet);

			NkCamera3DData cd;
			cd.position = position;
			cd.target = position + (q * NkVec3f{ 0.f, 0.f, -1.f });
			cd.up = q * NkVec3f{ 0.f, 1.f, 0.f };
			cd.fovY = 70.f;
			cd.aspect = (float32)devInit.width / (float32)devInit.height;
			cd.nearPlane = 0.05f;
			cd.farPlane = 100.f;

			NkSceneContext sctx;
			sctx.camera = NkCamera3D(cd);
			sctx.time = (float32)image / 60.f;
			r3d->SetIBLStrength(0.35f);

			NkLightDesc soleil;
			soleil.type = NkLightType::NK_DIRECTIONAL;
			soleil.direction = { -0.5f, -1.f, -0.35f };
			soleil.color = { 1.f, 0.96f, 0.88f };
			soleil.intensity = 3.f;
			soleil.castShadow = false;
			sctx.lights.PushBack(soleil);

			r3d->BeginScene(sctx);
			SoumettreLaSalle(r3d, renderer->GetMeshSystem());

			if (auto *graph = renderer->GetRenderGraph()) {
				graph->Execute(cmd);
			}

			if (auto *r2d = rEcran->GetRender2D()) {
				r2d->DrawImage(cible->GetColorResult(),
							   { 0.f, 0.f, (float32)devInit.width,
								 (float32)devInit.height });
			}

			rEcran->Present();
			rEcran->EndFrame();
		}

		++image;

		if (imageDeCapture != 0 && image == imageDeCapture) {
			const char *chemin = getenv("MASALLE_SHOT_PATH");
			if (chemin == nullptr || *chemin == '\0') {
				chemin = "cone.png";
			}
			const bool ok = cible->Capture(chemin);
			std::printf("[cone] capture image %llu -> %s (%s)\n",
						(unsigned long long)image, chemin, ok ? "ok" : "echec");
			std::fflush(stdout);
		}

		if (imageDeSortie != 0 && image >= imageDeSortie) {
			tourne = false;
		}
	}

	std::printf("[cone] fin : lacet %.2f deg, ordre %s, %llu images\n",
				(double)lacetDeg, ordreBon ? "lacet*tangage" : "tangage*lacet",
				(unsigned long long)image);

	NkRenderer::Destroy(renderer);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}
