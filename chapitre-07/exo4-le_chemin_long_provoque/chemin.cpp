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
} // namespace

int nkmain(const NkEntryState &state) {
    (void)state;

    const char *ttenv = getenv("MASALLE_TOUR");
    const double tourDeg = (ttenv && *ttenv) ? atof(ttenv) : 20.0;

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

    uint64 image = 0;
    bool tourne = true;
    NkChrono ch;
    uint64 lastTns = 0;
    float lacetRad = 0.f;
    NkQuatf previousOrientation;
    uint64 previousNowT = 0;
    bool firstOrient = true;
    const float64 kTwoPi = 2.0 * 3.141592653589793;
    const float64 kTargetFrameMs = 1000.0 / 60.0;

    while (tourne && fenetre.IsOpen()) {
        NkEvent *ev = nullptr;
        while (NkEvents().PollEvent(ev)) {
            if (ev->As<NkWindowCloseEvent>() != nullptr) {
                tourne = false;
            }
        }
        if (!tourne) break;

        const uint64 frameStart = ch.Now().nanoseconds;
        const uint64 nowT = ch.Now().nanoseconds;
        if (lastTns == 0) lastTns = nowT;
        const double dt = (nowT - lastTns) / 1e9;
        lacetRad += (float)(tourDeg * 3.141592653589793 / 180.0 * dt);
        lastTns = nowT;

        const NkMat4f rotation =
            NkMat4f::RotationY(NkAngle::FromRad(lacetRad)) *
            NkMat4f::RotationX(NkAngle::FromRad(0.f));
        const NkQuatf orientation = NkQuatf(rotation).Normalized();

        NkCamera3DData cd;
        cd.position = { 0.30f, kYeux, 1.20f };
        cd.target = cd.position + orientation * NkVec3f(0.f, 0.f, -1.f);
        cd.up = { 0.f, 1.f, 0.f };
        cd.fovY = 70.f;
        cd.aspect = (float32)devInit.width / (float32)devInit.height;
        cd.nearPlane = 0.05f;
        cd.farPlane = 100.f;

        const float32 dtSeconds = (float32)((double)(nowT - previousNowT) / 1e9);

        if (!firstOrient && dtSeconds > 0.f) {
            const double lacetDeg = lacetRad * 180.0 / 3.141592653589793;
            NkQuatf delta = (orientation * previousOrientation.Conjugate()).Normalized();
            // TEST RETIRE : pas de if (delta.w < 0.f) ici.
            const float32 angleRad = delta.Angle().Rad();
            NkVec3f vitesse;
            if (angleRad <= 1e-6f) {
                vitesse = NkVec3f(0.f, 0.f, 0.f);
            } else {
                vitesse = delta.Axis() * (angleRad / dtSeconds);
            }
            std::printf("image %llu : lacet = %.1f deg, vitesse = (%.1f, %.1f, %.1f) deg/s, delta.w = %.4f\n",
                        (unsigned long long)image, lacetDeg,
                        vitesse.x * 180.0 / 3.141592653589793,
                        vitesse.y * 180.0 / 3.141592653589793,
                        vitesse.z * 180.0 / 3.141592653589793,
                        delta.w);
            fflush(stdout);
        }

        previousOrientation = orientation;
        previousNowT = nowT;
        firstOrient = false;

        NkSceneContext sctx;
        sctx.camera = NkCamera3D(cd);
        sctx.time = (float32)image / 60.f;
        (void)renderer;

        if (rEcran->BeginFrame()) {
            rEcran->EndFrame();
        }

        const float64 elapsedMs = (double)(ch.Now().nanoseconds - frameStart) / 1e6;
        if (elapsedMs < kTargetFrameMs) {
            NkChrono::Sleep(kTargetFrameMs - elapsedMs);
        }

        ++image;
        if (lacetRad >= kTwoPi) {
            tourne = false;
        }
    }

    std::printf("[MaSalle] %llu images\n", (unsigned long long)image);

    NkRenderer::Destroy(renderer);
    NkRenderer::Destroy(rEcran);
    NkDeviceFactory::Destroy(device);
    return 0;
}
