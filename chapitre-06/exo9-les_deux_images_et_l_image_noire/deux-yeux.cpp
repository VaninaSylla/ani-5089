#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKEvent/NkEventSystem.h"
#include "NKEvent/NkWindowEvent.h"
#include "NKImage/NKImage.h"

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
using namespace nkentseu::math;
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

	const int32 kW = 640, kH = 720;	 // la paire 640×720
	const float32 kIPDmm = 63.f;	 // inter-distance pupillaire
	const NkVec3f kPosition = { 0.30f, kYeux, 1.20f }; // la tête, droite

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

	// Un œil : son renderer tourne la salle dans sa propre cible, sur la
	// file de commandes commune — les deux graphes s'enchaînent avant le
	// dessin côte à côte de rEcran.
	void RendreUnOeil(NkRenderer *renderer, NkICommandBuffer *cmd,
					  const NkVec3f &oeil, uint64 image) {
		renderer->FlushGraphRebuilds();
		NkRender3D *r3d = renderer->GetRender3D();
		r3d->ResetFrame();
		if (auto *mc = renderer->GetMaterialCollection()) {
			mc->Upload();
		}

		NkCamera3DData cd;
		cd.position = oeil;
		cd.target = oeil + NkVec3f{ 0.f, 0.f, -1.f };
		cd.up = { 0.f, 1.f, 0.f };
		cd.fovY = 70.f;
		cd.aspect = (float32)kW / (float32)kH;
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
	}

	// La comparaison, et l'image d'écart : écart = max des quatre canaux par
	// pixel ; compté s'il dépasse 1 ; l'image amplifie de 4× (borné 255).
	struct Comparaison {
		int64 pixels = 0;
		int64 total = 0;
		int32 max = 0;
	};

	Comparaison ComparerEtEcrire(const char *gauche, const char *droit,
								 const char *diffPng) {
		Comparaison res;
		NkImage imgG, imgD;
		if (!imgG.Load(gauche, 4) || !imgD.Load(droit, 4)) {
			std::printf("[yeux] impossible de relire %s et %s\n", gauche, droit);
			return res;
		}

		NkImage diff = NkImage::Create((uint32)kW, (uint32)kH, 4, 0u);
		for (int32 y = 0; y < kH; ++y) {
			for (int32 x = 0; x < kW; ++x) {
				const NkColor a = imgG.GetPixel(x, y);
				const NkColor b = imgD.GetPixel(x, y);
				int32 d = (int32)std::abs((int32)a.r - (int32)b.r);
				if ((int32)std::abs((int32)a.g - (int32)b.g) > d)
					d = (int32)std::abs((int32)a.g - (int32)b.g);
				if ((int32)std::abs((int32)a.b - (int32)b.b) > d)
					d = (int32)std::abs((int32)a.b - (int32)b.b);
				if ((int32)std::abs((int32)a.a - (int32)b.a) > d)
					d = (int32)std::abs((int32)a.a - (int32)b.a);
				++res.total;
				if (d > 1) ++res.pixels;
				if (d > res.max) res.max = d;
				const int32 v = d * 4 > 255 ? 255 : d * 4;
				diff.SetPixel(x, y, NkColor((uint8)v, (uint8)v, (uint8)v, 255));
			}
		}
		if (!diff.SavePNG(diffPng)) {
			std::printf("[yeux] impossible d'ecrire %s\n", diffPng);
		}
		return res;
	}

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig cfg;
	cfg.title = "Ma salle — deux yeux";
	cfg.width = 1280;
	cfg.height = 720;
	cfg.centered = true;
	cfg.vsync = true;

	NkWindow fenetre(cfg);
	if (!fenetre.IsValid()) {
		std::printf("[yeux] la fenetre n'a pas pu s'ouvrir\n");
		return 1;
	}

	NkDeviceInitInfo devInit{};
	devInit.surface = fenetre.GetSurfaceDesc();
	devInit.width = fenetre.GetSize().x;
	devInit.height = fenetre.GetSize().y;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInit);
	if (device == nullptr) {
		std::printf("[yeux] aucune API graphique disponible\n");
		return 1;
	}

	NkRendererConfig cfgEcran =
		NkRendererConfig::For2D(devInit.api, devInit.width, devInit.height);
	NkRenderer *rEcran = NkRenderer::Create(device, cfgEcran);
	if (rEcran == nullptr || !rEcran->Initialize()) {
		std::printf("[yeux] le renderer d'ecran n'a pas pu s'initialiser\n");
		NkDeviceFactory::Destroy(device);
		return 1;
	}

	// Deux cibles 640×720, une par œil — lues pour la comparaison, lues
	// aussi pour le dessin côte à côte.
	NkOffscreenDesc od;
	od.width = kW;
	od.height = kH;
	od.hasDepth = true;
	od.hdr = false;
	od.colorFmt = NkGPUFormat::NK_RGBA8_UNORM;
	od.readable = true;
	od.readback = true;
	od.name = "Oeil";

	NkOffscreenTarget *cibleG = rEcran->CreateOffscreen(od);
	NkOffscreenTarget *cibleD = rEcran->CreateOffscreen(od);
	if (cibleG == nullptr || cibleD == nullptr) {
		std::printf("[yeux] cibles hors ecran refusees\n");
		NkRenderer::Destroy(rEcran);
		NkDeviceFactory::Destroy(device);
		return 1;
	}

	// Deux renderers de salle : chacun rend dans SA cible.
	NkRendererConfig cfgSalle =
		NkRendererConfig::ForGame(devInit.api, kW, kH);
	cfgSalle.Disable(NK_SS_RENDER2D);
	cfgSalle.Disable(NK_SS_TEXT);
	cfgSalle.Disable(NK_SS_OVERLAY);
	cfgSalle.Enable(NK_SS_OFFSCREEN);
	if (EnvU64("MASALLE_POST", 1) == 0) {
		cfgSalle.Disable(NK_SS_POST_PROCESS);
	}

	NkRenderer *rendererG = NkRenderer::Create(device, cfgSalle);
	NkRenderer *rendererD = NkRenderer::Create(device, cfgSalle);
	if (rendererG == nullptr || !rendererG->Initialize() ||
		rendererD == nullptr || !rendererD->Initialize()) {
		std::printf("[yeux] les renderers des yeux n'ont pas pu s'initialiser\n");
		NkRenderer::Destroy(rendererG);
		NkRenderer::Destroy(rendererD);
		NkRenderer::Destroy(rEcran);
		NkDeviceFactory::Destroy(device);
		return 1;
	}
	rendererG->SetFinalColorTarget(
		rEcran->GetTextures()->GetRHIHandle(cibleG->GetColorResult()));
	rendererG->SetRenderSizeOverride((uint32)kW, (uint32)kH);
	rendererD->SetFinalColorTarget(
		rEcran->GetTextures()->GetRHIHandle(cibleD->GetColorResult()));
	rendererD->SetRenderSizeOverride((uint32)kW, (uint32)kH);

	std::printf("[yeux] deux cibles %dx%d, IPD %.1f mm a l'image 30, puis "
				"IPD nul (vue oubliee) a l'image 60\n",
				kW, kH, (double)kIPDmm);
	std::fflush(stdout);

	const uint64 imageDeSortie = EnvU64("MASALLE_EXIT", 0);

	uint64 image = 0;
	bool tourne = true;
	bool phaseOublie = false;

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

			// La vue oubliée : les deux caméras au même point.
			const float32 demi = (phaseOublie ? 0.f : kIPDmm * 0.5f * 0.001f);
			const NkVec3f oeilG = kPosition - NkVec3f{ demi, 0.f, 0.f };
			const NkVec3f oeilD = kPosition + NkVec3f{ demi, 0.f, 0.f };

			RendreUnOeil(rendererG, cmd, oeilG, image);
			RendreUnOeil(rendererD, cmd, oeilD, image);

			if (auto *r2d = rEcran->GetRender2D()) {
				r2d->DrawImage(cibleG->GetColorResult(),
							   { 0.f, 0.f, (float32)kW, (float32)kH });
				r2d->DrawImage(cibleD->GetColorResult(),
							   { (float32)kW, 0.f, (float32)kW, (float32)kH });
			}

			rEcran->Present();
			rEcran->EndFrame();
		}

		++image;

		if (!phaseOublie && image == 30) {
			const bool okG = cibleG->Capture("oeil-gauche.png");
			const bool okD = cibleD->Capture("oeil-droit.png");
			std::printf("[yeux] captures (IPD %.1f mm) : %s %s\n", (double)kIPDmm,
						okG ? "ok" : "echec", okD ? "ok" : "echec");
			if (okG && okD) {
				const Comparaison c = ComparerEtEcrire(
					"oeil-gauche.png", "oeil-droit.png", "difference.png");
				std::printf("deux yeux : %lld pixels sur %lld, ecart max %d\n",
							(long long)c.pixels, (long long)c.total, c.max);
			}
			std::fflush(stdout);
			phaseOublie = true; // les images suivantes : IPD nul
		}

		if (phaseOublie && image == 60) {
			const bool okG = cibleG->Capture("oeil-gauche-oublie.png");
			const bool okD = cibleD->Capture("oeil-droit-oublie.png");
			std::printf("[yeux] captures (IPD 0) : %s %s\n",
						okG ? "ok" : "echec", okD ? "ok" : "echec");
			if (okG && okD) {
				const Comparaison c = ComparerEtEcrire(
					"oeil-gauche-oublie.png", "oeil-droit-oublie.png",
					"difference-vue-oubliee.png");
				std::printf("vue oubliee : %lld pixels sur %lld, ecart max %d\n",
							(long long)c.pixels, (long long)c.total, c.max);
			}
			std::remove("oeil-gauche-oublie.png");
			std::remove("oeil-droit-oublie.png");
			std::fflush(stdout);
			tourne = false;
		}

		if (imageDeSortie != 0 && image >= imageDeSortie) {
			tourne = false;
		}
	}

	std::printf("[yeux] %llu images\n", (unsigned long long)image);

	NkRenderer::Destroy(rendererG);
	NkRenderer::Destroy(rendererD);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}
