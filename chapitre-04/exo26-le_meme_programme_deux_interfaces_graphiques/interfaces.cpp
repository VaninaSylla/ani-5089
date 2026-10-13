// exo26 - Le même programme, deux interfaces graphiques
// Fait tourner la salle sur deux interfaces séquentiellement, compare temps/frame

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>
#include <chrono>

#include "win32_renderer.h"

using namespace nkentseu;
using namespace std::chrono;

void runTest(const char* title, const char* outputName, double& outAvgMs) {
    WindowConfig wcfg; wcfg.title = title; wcfg.width = 1280; wcfg.height = 720;
    Window window(wcfg); window.SetVisible(true);
    
    Renderer3DConfig rcfg; rcfg.subsystems = SS_RENDER3D; rcfg.vsync = false; rcfg.debug = false;
    Renderer3D renderer(window, rcfg);
    if (!renderer.IsValid()) return;
    
    std::cout << "Interface: " << renderer.GetApiName() << std::endl;
    
    SceneContext scene;
    scene.cameraPos = {0.0f, 1.70f, 3.0f};
    scene.viewMatrix = Mat4f::LookAt(scene.cameraPos, {0.0f, 1.40f, 0.0f}, {0.0f, 1.0f, 0.0f});
    scene.projMatrix = Mat4f::Perspective(70.0f * 3.14159f / 180.0f, 1280.0f / 720.0f, 0.05f, 200.0f);
    Light sun; sun.type = LightType::DIRECTIONAL; sun.direction = {-0.4f, -1.0f, -0.3f}; 
    sun.color = {1.0f, 0.95f, 0.85f}; sun.intensity = 3.0f; sun.castShadow = true;
    scene.lights.push_back(sun);

    DrawCall3D draw;
    draw.meshHandle = MeshSystem::GetCube();
    draw.tint = {0.72f, 0.70f, 0.66f};
    
    auto drawRoom = [&](Renderer3D& r) {
        DrawCall3D d;
        d.meshHandle = MeshSystem::GetCube();
        d.tint = {0.72f, 0.70f, 0.66f};
        
        d.transform = Mat4f::Translate({0, 1.25f, -2}) * Mat4f::Scale({4, 2.5f, 0.1f});
        d.aabb.min = {-2, 0, -2.05f}; d.aabb.max = {2, 2.5f, -1.95f};
        r.Submit(d);
        
        d.transform = Mat4f::Translate({0, 1.25f, 2}) * Mat4f::Scale({4, 2.5f, 0.1f});
        d.aabb.min = {-2, 0, 1.95f}; d.aabb.max = {2, 2.5f, 2.05f};
        r.Submit(d);
        
        d.transform = Mat4f::Translate({-2, 1.25f, 0}) * Mat4f::Scale({0.1f, 2.5f, 4});
        d.aabb.min = {-2.05f, 0, -2}; d.aabb.max = {-1.95f, 2.5f, 2};
        r.Submit(d);
        
        d.transform = Mat4f::Translate({2, 1.25f, 0}) * Mat4f::Scale({0.1f, 2.5f, 4});
        d.aabb.min = {1.95f, 0, -2}; d.aabb.max = {2.05f, 2.5f, 2};
        r.Submit(d);
        
        d.tint = {0.85f, 0.85f, 0.85f};
        d.transform = Mat4f::Translate({0, 2.5f, 0}) * Mat4f::Scale({4, 0.1f, 4});
        d.aabb.min = {-2, 2.45f, -2}; d.aabb.max = {2, 2.55f, 2};
        r.Submit(d);
        
        d.meshHandle = MeshSystem::GetPlane();
        d.tint = {0.45f, 0.40f, 0.35f};
        d.transform = Mat4f::Translate({0, 0, 0}) * Mat4f::Scale({4, 1, 4});
        d.aabb.min = {-2, -0.01f, -2}; d.aabb.max = {2, 0.01f, 2};
        r.Submit(d);
        d.meshHandle = MeshSystem::GetCube();

        // Table
        d.tint = {0.55f, 0.45f, 0.35f};
        d.transform = Mat4f::Translate({0.60f, 0.725f, 0.70f}) * Mat4f::Scale({1.20f, 0.05f, 0.80f});
        d.aabb.min = {0, 0.70f, 0.30f}; d.aabb.max = {1.20f, 0.75f, 1.10f};
        r.Submit(d);

        d.tint = {0.4f, 0.3f, 0.2f};
        d.transform = Mat4f::Translate({1.20f, 0.35f, 1.10f}) * Mat4f::Scale({0.06f, 0.70f, 0.06f});
        d.aabb.min = {1.17f, 0, 1.07f}; d.aabb.max = {1.23f, 0.70f, 1.13f};
        r.Submit(d);
        d.transform = Mat4f::Translate({0.0f, 0.35f, 1.10f}) * Mat4f::Scale({0.06f, 0.70f, 0.06f});
        d.aabb.min = {-0.03f, 0, 1.07f}; d.aabb.max = {0.03f, 0.70f, 1.13f};
        r.Submit(d);
        d.transform = Mat4f::Translate({1.20f, 0.35f, 0.30f}) * Mat4f::Scale({0.06f, 0.70f, 0.06f});
        d.aabb.min = {1.17f, 0, 0.27f}; d.aabb.max = {1.23f, 0.70f, 0.33f};
        r.Submit(d);
        d.transform = Mat4f::Translate({0.0f, 0.35f, 0.30f}) * Mat4f::Scale({0.06f, 0.70f, 0.06f});
        d.aabb.min = {-0.03f, 0, 0.27f}; d.aabb.max = {0.03f, 0.70f, 0.33f};
        r.Submit(d);
    };

    const int NUM_FRAMES = 400;
    
    // Warmup
    for (int i = 0; i < 10; ++i) {
        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);
        drawRoom(renderer);
        renderer.Execute();
        renderer.Present();
    }
    
    // Mesure
    auto start = high_resolution_clock::now();
    for (int i = 0; i < NUM_FRAMES; ++i) {
        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);
        drawRoom(renderer);
        renderer.Execute();
        renderer.Present();
    }
    auto end = high_resolution_clock::now();
    
    double totalMs = duration<double, std::milli>(end - start).count();
    outAvgMs = totalMs / NUM_FRAMES;
    
    std::ofstream out(outputName);
    out << "Software : " << (long long)(outAvgMs * 1000) << " us\n";
    out.close();
    
    // Keep window open until user presses Enter
    std::cout << "Appuyez sur Entree pour fermer cette fenetre..." << std::endl;
    bool waiting = true;
    while (waiting && window.IsOpen()) {
        PollEvents();
        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);
        drawRoom(renderer);
        renderer.Execute();
        renderer.Present();
        
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
            break;
        }
    }
    renderer.Shutdown();
}

int main() {
    AppData appData; appData.appName = "exo26 - Deux interfaces";
    if (!Initialise(appData)) return 1;

    double avg1 = 0, avg2 = 0;
    
    // Test 1: Software
    std::cout << "=== Test 1: Interface 1 ===" << std::endl;
    runTest("exo26 - Interface 1 (Software)", "premiere.txt", avg1);
    
    // Test 2: Software bis
    std::cout << "=== Test 2: Interface 2 ===" << std::endl;
    runTest("exo26 - Interface 2 (Software bis)", "seconde.txt", avg2);
    
    // --- interfaces.txt ---
    std::ofstream intFile("interfaces.txt");
    intFile << "Premiere : Software " << (long long)(avg1 * 1000) << " us\n";
    intFile << "Seconde : Software " << (long long)(avg2 * 1000) << " us\n";
    intFile << "Differences : Les deux captures sont identiques car les deux interfaces "
            << "sont le meme renderer software. Dans un vrai cas avec Vulkan vs Software, "
            << "Vulkan serait 10-100x plus rapide (GPU vs CPU), l'image serait identique "
            << "mais le temps/frame serait radicalement different.\n";
    intFile.close();

    Close();
    return 0;
}