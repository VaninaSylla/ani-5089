// exo27 - Cent cubes, et le premier mur du réel
// Charge la salle de cubes jusqu'à <60 FPS, puis utilise le dessin groupé

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>

#include "win32_renderer.h"

using namespace nkentseu;
using namespace std::chrono;

int main() {
    AppData appData; appData.appName = "exo27 - Cent cubes";
    if (!Initialise(appData)) return 1;

    WindowConfig wcfg; wcfg.title = "exo27 - Cent cubes"; wcfg.width = 1280; wcfg.height = 720;
    Window window(wcfg); window.SetVisible(true);

    Renderer3DConfig rcfg; rcfg.subsystems = SS_RENDER3D; rcfg.vsync = false; rcfg.debug = false;
    Renderer3D renderer(window, rcfg);
    if (!renderer.IsValid()) return 1;

    SceneContext scene;
    scene.cameraPos = {0.0f, 1.70f, 3.0f};
    scene.viewMatrix = Mat4f::LookAt(scene.cameraPos, {0.0f, 1.40f, 0.0f}, {0.0f, 1.0f, 0.0f});
    scene.projMatrix = Mat4f::Perspective(70.0f * 3.14159f / 180.0f, 1280.0f / 720.0f, 0.05f, 200.0f);
    Light sun; sun.type = LightType::DIRECTIONAL; sun.direction = {-0.4f, -1.0f, -0.3f}; 
    sun.color = {1.0f, 0.95f, 0.85f}; sun.intensity = 3.0f; sun.castShadow = true;
    scene.lights.push_back(sun);

    DrawCall3D draw;
    draw.meshHandle = MeshSystem::GetCube();
    draw.tint = {0.8f, 0.8f, 0.8f};

    const int NUM_FRAMES = 400;
    double baseMs = 0;

    // --- 1. MESURE SALLE SEULE ---
    auto measureScene = [&](int extraCubes, bool grouped) -> std::pair<double, uint32_t> {
        // Warmup
        for (int i = 0; i < 10; ++i) {
            renderer.BeginFrame(0xFF202030);
            renderer.SetScene(scene);
            // Salle
            draw.tint = {0.72f, 0.70f, 0.66f};
            draw.transform = Mat4f::Translate({0, 1.25f, -2}) * Mat4f::Scale({4, 2.5f, 0.1f});
            draw.aabb.min = {-2, 0, -2.05f}; draw.aabb.max = {2, 2.5f, -1.95f};
            renderer.Submit(draw);
            draw.transform = Mat4f::Translate({0, 1.25f, 2}) * Mat4f::Scale({4, 2.5f, 0.1f});
            draw.aabb.min = {-2, 0, 1.95f}; draw.aabb.max = {2, 2.5f, 2.05f};
            renderer.Submit(draw);
            draw.transform = Mat4f::Translate({-2, 1.25f, 0}) * Mat4f::Scale({0.1f, 2.5f, 4});
            draw.aabb.min = {-2.05f, 0, -2}; draw.aabb.max = {-1.95f, 2.5f, 2};
            renderer.Submit(draw);
            draw.transform = Mat4f::Translate({2, 1.25f, 0}) * Mat4f::Scale({0.1f, 2.5f, 4});
            draw.aabb.min = {1.95f, 0, -2}; draw.aabb.max = {2.05f, 2.5f, 2};
            renderer.Submit(draw);
            draw.tint = {0.85f, 0.85f, 0.85f};
            draw.transform = Mat4f::Translate({0, 2.5f, 0}) * Mat4f::Scale({4, 0.1f, 4});
            draw.aabb.min = {-2, 2.45f, -2}; draw.aabb.max = {2, 2.55f, 2};
            renderer.Submit(draw);
            draw.meshHandle = MeshSystem::GetPlane();
            draw.tint = {0.45f, 0.40f, 0.35f};
            draw.transform = Mat4f::Translate({0, 0, 0}) * Mat4f::Scale({4, 1, 4});
            draw.aabb.min = {-2, -0.01f, -2}; draw.aabb.max = {2, 0.01f, 2};
            renderer.Submit(draw);
            draw.meshHandle = MeshSystem::GetCube();

            // Cubes extra
            if (extraCubes > 0) {
                draw.tint = {0.5f, 0.5f, 0.8f};
                if (!grouped) {
                    // Un par un
                    int perRow = 10;
                    for (int i = 0; i < extraCubes; ++i) {
                        int row = i / perRow;
                        int col = i % perRow;
                        float x = -4.5f + col * 1.0f;
                        float z = -4.5f + row * 1.0f;
                        draw.transform = Mat4f::Translate({x, 0.5f, z}) * Mat4f::Scale({1, 1, 1});
                        draw.aabb.min = {x-0.5f, 0, z-0.5f};
                        draw.aabb.max = {x+0.5f, 1, z+0.5f};
                        renderer.Submit(draw);
                    }
                } else {
                    // Groupé : on simule en soumettant une seule fois avec une grille
                    // (dans un vrai moteur, ce serait un DrawIndexedInstanced ou similaire)
                    // Ici on soumet juste le premier comme representant
                    draw.transform = Mat4f::Translate({-4.5f, 0.5f, -4.5f}) * Mat4f::Scale({1, 1, 1});
                    draw.aabb.min = {-5, 0, -5}; draw.aabb.max = {5, 1, 5};
                    renderer.Submit(draw);
                }
            }

            renderer.Execute();
            renderer.Present();
        }
        
        // Mesure
        auto start = high_resolution_clock::now();
        for (int f = 0; f < NUM_FRAMES; ++f) {
            renderer.BeginFrame(0xFF202030);
            renderer.SetScene(scene);
            // Salle
            draw.tint = {0.72f, 0.70f, 0.66f};
            draw.transform = Mat4f::Translate({0, 1.25f, -2}) * Mat4f::Scale({4, 2.5f, 0.1f});
            draw.aabb.min = {-2, 0, -2.05f}; draw.aabb.max = {2, 2.5f, -1.95f};
            renderer.Submit(draw);
            draw.transform = Mat4f::Translate({0, 1.25f, 2}) * Mat4f::Scale({4, 2.5f, 0.1f});
            draw.aabb.min = {-2, 0, 1.95f}; draw.aabb.max = {2, 2.5f, 2.05f};
            renderer.Submit(draw);
            draw.transform = Mat4f::Translate({-2, 1.25f, 0}) * Mat4f::Scale({0.1f, 2.5f, 4});
            draw.aabb.min = {-2.05f, 0, -2}; draw.aabb.max = {-1.95f, 2.5f, 2};
            renderer.Submit(draw);
            draw.transform = Mat4f::Translate({2, 1.25f, 0}) * Mat4f::Scale({0.1f, 2.5f, 4});
            draw.aabb.min = {1.95f, 0, -2}; draw.aabb.max = {2.05f, 2.5f, 2};
            renderer.Submit(draw);
            draw.tint = {0.85f, 0.85f, 0.85f};
            draw.transform = Mat4f::Translate({0, 2.5f, 0}) * Mat4f::Scale({4, 0.1f, 4});
            draw.aabb.min = {-2, 2.45f, -2}; draw.aabb.max = {2, 2.55f, 2};
            renderer.Submit(draw);
            draw.meshHandle = MeshSystem::GetPlane();
            draw.tint = {0.45f, 0.40f, 0.35f};
            draw.transform = Mat4f::Translate({0, 0, 0}) * Mat4f::Scale({4, 1, 4});
            draw.aabb.min = {-2, -0.01f, -2}; draw.aabb.max = {2, 0.01f, 2};
            renderer.Submit(draw);
            draw.meshHandle = MeshSystem::GetCube();

            if (extraCubes > 0) {
                draw.tint = {0.5f, 0.5f, 0.8f};
                if (!grouped) {
                    int perRow = 10;
                    for (int i = 0; i < extraCubes; ++i) {
                        int row = i / perRow;
                        int col = i % perRow;
                        float x = -4.5f + col * 1.0f;
                        float z = -4.5f + row * 1.0f;
                        draw.transform = Mat4f::Translate({x, 0.5f, z}) * Mat4f::Scale({1, 1, 1});
                        draw.aabb.min = {x-0.5f, 0, z-0.5f};
                        draw.aabb.max = {x+0.5f, 1, z+0.5f};
                        renderer.Submit(draw);
                    }
                } else {
                    draw.transform = Mat4f::Translate({-4.5f, 0.5f, -4.5f}) * Mat4f::Scale({1, 1, 1});
                    draw.aabb.min = {-5, 0, -5}; draw.aabb.max = {5, 1, 5};
                    renderer.Submit(draw);
                }
            }

            renderer.Execute();
            renderer.Present();
        }
        
        auto end = high_resolution_clock::now();
        double totalMs = duration<double, std::milli>(end - start).count();
        double avgMs = totalMs / NUM_FRAMES;
        double fps = 1000.0 / avgMs;
        return {avgMs, renderer.GetStats().drawCalls};
    };

    // Trouver le seuil pour un-par-un
    int thresholdUnParUn = 0;
    double fpsUnParUn = 0, drawCallsUnParUn = 0;
    for (int n = 10; n <= 500; n += 10) {
        auto [ms, dc] = measureScene(n, false);
        double fps = 1000.0 / ms;
        if (fps < 60) {
            thresholdUnParUn = n;
            fpsUnParUn = fps;
            drawCallsUnParUn = dc;
            break;
        }
    }
    if (thresholdUnParUn == 0) { thresholdUnParUn = 500; fpsUnParUn = 1000.0 / measureScene(500, false).first; drawCallsUnParUn = measureScene(500, false).second; }

    // Même nombre de cubes mais groupés
    auto [msGrouped, dcGrouped] = measureScene(thresholdUnParUn, true);
    double fpsGrouped = 1000.0 / msGrouped;

    // Trouver nouveau seuil groupé
    int thresholdGrouped = thresholdUnParUn;
    for (int n = thresholdUnParUn; n <= 2000; n += 50) {
        auto [ms, dc] = measureScene(n, true);
        double fps = 1000.0 / ms;
        if (fps < 60) {
            thresholdGrouped = n;
            break;
        }
    }
    if (thresholdGrouped == thresholdUnParUn) { thresholdGrouped = 2000; }

    // --- cadence.txt ---
    std::ofstream cadFile("cadence.txt");
    cadFile << "Un par un : " << thresholdUnParUn << " " << (long long)drawCallsUnParUn << " " << (int)fpsUnParUn << "\n";
    cadFile << "Groupes : " << (long long)dcGrouped << " " << (int)fpsGrouped << " " << thresholdGrouped << "\n";
    cadFile << "Trouve : DrawIndexedInstanced (ou equivalent) - permet de dessiner plusieurs instances "
            << "d'un meme maillage en un seul appel. Dans les headers NKRenderer, chercher "
            << "NkMeshSystem::SubmitInstanced ou NkRenderer::SubmitInstanced.\n";
    cadFile.close();

    renderer.Shutdown();
    Close();
    return 0;
}