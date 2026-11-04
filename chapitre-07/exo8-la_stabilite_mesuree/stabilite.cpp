#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKEvent/NkEventSystem.h"
#include "NKEvent/NkWindowEvent.h"
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
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cmath>
#include "NKTime/NkChrono.h"
#include "NKEvent/NkMouseEvent.h"
#include "NKMath/NkQuat.h"

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

uint64 EnvU64(const char *nom, uint64 defaut) {
    const char *t = getenv(nom);
    if (t == nullptr || *t == '\0') return defaut;
    return (uint64)strtoull(t, nullptr, 10);
}

struct Mesure {
    uint64 count;
    double mean, stddev, maxv;
    uint64 overBudget;
};

Mesure MesurerUneMinute(NkChrono &ch, NkWindow &fenetre, NkRenderer *rEcran,
                        NkRenderer *renderer, NkOffscreenTarget *cible,
                        const NkDeviceInitInfo &devInit, bool avecAttente) {
    std::vector<double> deltas;
    const double budgetMs = 1000.0 / 90.0;
    uint64 overBudget = 0;
    uint64 image = 0;
    bool tourne = true;
    uint64 last_ns = ch.Now().nanoseconds;
    int seen = 0;

    const double minuteNs = 60.0 * 1e9;
    const uint64 stopNs = last_ns + (uint64)minuteNs;

    while (tourne && fenetre.IsOpen()) {
        NkEvent *ev = nullptr;
        while (NkEvents().PollEvent(ev)) {
            if (ev->As<NkWindowCloseEvent>() != nullptr) {
                tourne = false;
            }
        }
        if (!tourne) break;

        uint64 now_ns = ch.Now().nanoseconds;
        if (now_ns >= stopNs) {
            tourne = false;
            break;
        }

        if (seen >= 30) {
            double msd = (now_ns - last_ns) / 1000000.0;
            if (msd > 0.0) {
                deltas.push_back(msd);
                if (msd > budgetMs) ++overBudget;
            }
        }
        last_ns = now_ns;
        ++seen;

        if (rEcran->BeginFrame()) {
            NkICommandBuffer *cmd = rEcran->GetCmd();
            NkRender3D *r3d = renderer->GetRender3D();
            renderer->FlushGraphRebuilds();
            r3d->ResetFrame();
            if (auto *mc = renderer->GetMaterialCollection()) {
                mc->Upload();
            }
            NkCamera3DData cd;
            cd.position = { 0.30f, kYeux, 1.20f };
            cd.target = { -0.70f, 1.10f, -kDemi };
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
    }

    Mesure m;
    m.count = (uint64)deltas.size();
    if (m.count > 0) {
        double sum = 0;
        for (auto x : deltas) sum += x;
        m.mean = sum / m.count;
        double sq = 0;
        for (auto x : deltas) {
            double d = x - m.mean;
            sq += d * d;
        }
        m.stddev = std::sqrt(sq / m.count);
        m.maxv = *std::max_element(deltas.begin(), deltas.end());
    } else {
        m.mean = m.stddev = m.maxv = 0;
    }
    m.overBudget = overBudget;
    return m;
}
} // namespace

int nkmain(const NkEntryState &state) {
    (void)state;

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

    devInit.api = NkGraphicsApi::NK_GFX_API_OPENGL;
    NkIDevice *device = NkDeviceFactory::CreateForApi(devInit.api, devInit);
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

    NkChrono ch;

    // 1ère mesure : salle telle quelle (attente de l'ecran) ou sans attente
    // selon NK_VK_MAILBOX (une minute exactement, demarrage ecarte).
    Mesure m1 = MesurerUneMinute(ch, fenetre, rEcran, renderer, cible, devInit,
                                 false);

    const char *label = (getenv("NK_VK_MAILBOX") != nullptr) ? "sans attente" : "ecran";
    std::printf("%s : %llu images, moyenne %.2f ms, ecart type %.2f ms, max %.2f ms, hors budget %llu\n",
                label, (unsigned long long)m1.count, m1.mean, m1.stddev, m1.maxv,
                (unsigned long long)m1.overBudget);
    fflush(stdout);

    std::printf("[MaSalle] fin mesure %s\n", label);

    NkRenderer::Destroy(renderer);
    NkRenderer::Destroy(rEcran);
    NkDeviceFactory::Destroy(device);
    return 0;
}
