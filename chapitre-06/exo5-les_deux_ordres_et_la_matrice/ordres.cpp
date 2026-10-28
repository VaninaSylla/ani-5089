// =============================================================================
//  Sprint 6, exercice 5 — les deux ordres et la matrice.
//
//  Une tête qui tourne et lève les yeux se décompose en deux rotations :
//  un lacet (autour de +Y) et un tangage (autour de +X). Les combiner en
//  quaternion, c'est les multiplier — mais « l'ordre de lecture d'un produit
//  de quaternions dépend de la bibliothèque » : qLacet * qTangage et
//  qTangage * qLacet ne sont pas la même rotation, et leurs deux regards
//  ne sont pas le même regard. Pour départager les deux, on fait le même
//  calcul avec des MATRICES (NkMat4f::RotationY * NkMat4f::RotationX), dont
//  l'ordre de composition, lui, ne se discute pas : (A * B) applique B
//  d'abord aux vecteurs colonnes. On passe la matrice au quaternion avec
//  NkQuatf(rotation).Normalized() et on regarde lequel des deux produits se
//  rejoint.
//
//  Au démarrage, AVANT d'ouvrir la fenêtre : les trois regards (lacet*tangage,
//  tangage*lacet, matrices) sont imprimés à quatre décimales — à recopier
//  dans regards.txt — plus la composante y calculée à la main.
//
//  Le calcul à la main : lever les yeux de 30°, c'est tourner le regard de
//  30° autour de +X ; la hauteur gagnée est sin(30°) — et sin(30°) = 1/2
//  exactement, quel que soit le lacet (une rotation autour de +Y ne touche
//  jamais à y). On écrit 0.5f en littéral : c'est un attendu, pas un calcul
//  qui reposerait sur le code sous examen. Aux trois impressions on confronte
//  donc cette valeur : le bon ordre donnera y = 0.5000, l'autre s'en
//  écartera — l'écart est la réponse, pas un accident d'arrondi.
//
//  Note sur les commentaires du moteur : NkQuat.h explique le sens du produit
//  à deux endroits (« Convention : 'other' est appliqué AVANT 'this' », près
//  de l'opérateur, et la formule elle-même) — et son histoire en dit long :
//  le commentaire près de l'opérateur disait autrefois « APRÈS » pendant que
//  le code calculait « AVANT » (relevé dans le fichier, 08/10). Une telle
//  divergence suffit : le calcul ci-dessus tranche, pas la prose.
//
//  Départ : la salle complète du kit (etapes/etape5_salle.cpp).
// =============================================================================
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKEvent/NkEventSystem.h"
#include "NKEvent/NkWindowEvent.h"
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

	uint64 EnvU64(const char *nom, uint64 defaut) {
		const char *t = getenv(nom);
		if (t == nullptr || *t == '\0') return defaut;
		return (uint64)strtoull(t, nullptr, 10);
	}

	// ── Les trois regards ─────────────────────────────────────────────────
	// Calculés au démarrage AVANT la fenêtre. Les attendus ne sont pas écrits
	// ici : c'est l'impression elle-même qui sera confrontée au calcul à la
	// main de y (sin 30° = 1/2), puis recopiée dans regards.txt.
	NkVec3f LancerLesTroisRegards() {
		const float32 lacetDeg = 45.f;
		const float32 tangageDeg = 30.f;

		const NkQuatf qLacet = NkQuatf::RotateY(NkAngle(lacetDeg));
		const NkQuatf qTangage = NkQuatf::RotateX(NkAngle(tangageDeg));
		const NkVec3f avant = { 0.f, 0.f, -1.f };

		const NkVec3f regardLacetTangage = (qLacet * qTangage) * avant;
		const NkVec3f regardTangageLacet = (qTangage * qLacet) * avant;

		const NkMat4f rotation = NkMat4f::RotationY(NkAngle(lacetDeg)) *
								 NkMat4f::RotationX(NkAngle(tangageDeg));
		const NkQuatf qMatrice = NkQuatf(rotation).Normalized();
		const NkVec3f regardMatrice = qMatrice * avant;

		// Calcul à la main : les yeux levés de 30°, la hauteur du regard vaut
		// sin(30°) = 1/2 — indépendamment du lacet, une rotation autour de +Y
		// ne change pas y. Écrit en littéral : c'est l'attendu qu'on affronte
		// aux trois impressions, pas un second passage par le code testé.
		const float32 yMain = 0.5f;

		std::printf("lacet*tangage : (%.4f, %.4f, %.4f)\n",
					(double)regardLacetTangage.x, (double)regardLacetTangage.y,
					(double)regardLacetTangage.z);
		std::printf("tangage*lacet : (%.4f, %.4f, %.4f)\n",
					(double)regardTangageLacet.x, (double)regardTangageLacet.y,
					(double)regardTangageLacet.z);
		std::printf("matrices : (%.4f, %.4f, %.4f)\n",
					(double)regardMatrice.x, (double)regardMatrice.y,
					(double)regardMatrice.z);
		std::printf("y a la main : %.4f\n", (double)yMain);
		std::fflush(stdout);

		return regardMatrice;
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

	// ── Au démarrage, AVANT la fenêtre ────────────────────────────────────
	const NkVec3f regardMatrice = LancerLesTroisRegards();

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

	std::printf("[MaSalle] salle de %.1f m sur %.1f m, plafond a %.2f m\n",
				(double)kPiece, (double)kPiece, (double)kPlafond);
	std::fflush(stdout);

	const uint64 imageDeSortie = EnvU64("MASALLE_EXIT", 0);
	const uint64 imageDeCapture = EnvU64("MASALLE_SHOT", 0);

	// La camera regarde le long du regard obtenu par le chemin matriciel :
	// c'est l'ordre que la salle « tient » dans sa pose.
	const NkVec3f position = { 0.30f, kYeux, 1.20f };

	uint64 image = 0;
	bool tourne = true;

	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr) {
				tourne = false;
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

			NkCamera3DData cd;
			cd.position = position;
			cd.target = position + regardMatrice;
			cd.up = { 0.f, 1.f, 0.f };
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
				chemin = "masalle.png";
			}
			const bool ok = cible->Capture(chemin);
			std::printf("[MaSalle] capture image %llu -> %s (%s)\n",
						(unsigned long long)image, chemin, ok ? "ok" : "echec");
		}

		if (imageDeSortie != 0 && image >= imageDeSortie) {
			tourne = false;
		}
	}

	std::printf("[MaSalle] %llu images\n", (unsigned long long)image);

	NkRenderer::Destroy(renderer);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}
