// exo10 - Ce que vos sous-systèmes coûtent vraiment
// Mesure le coût réel de chaque sous-système sur 400 frames.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

#include "win32_renderer.h"

using namespace nkentseu;
using namespace std::chrono;

int main() {
    AppData appData; appData.appName = "exo10 - Cout sous-systemes";
    if (!Initialise(appData)) return 1;

    WindowConfig wcfg; wcfg.title = "exo10 - Mesure"; wcfg.width = 1280; wcfg.height = 720;
    Window window(wcfg); window.SetVisible(true);

    // Sous-systèmes à tester (nom, bitmask)
    struct SubsysTest { const char* name; RendererSubsystem mask; };
    SubsysTest tests[] = {
        {"RENDER3D", SS_RENDER3D},
        {"RENDER3D+SHADOW", SS_RENDER3D | SS_SHADOW},
        {"RENDER3D+RAYTRACE", SS_RENDER3D | SS_RAYTRACE},
        {"RENDER3D+VISIBILITY", SS_RENDER3D | SS_VISIBILITY},
        {"RENDER3D+SHADOW+RAYTRACE", SS_RENDER3D | SS_SHADOW | SS_RAYTRACE},
        {"ALL", SS_ALL},
    };
    
    const int NUM_FRAMES = 400;
    std::ofstream mesuresFile("mesures.txt");
    std::ofstream classementFile("classement.txt");
    std::ofstream choixFile("choix.txt");

    struct Result { const char* name; double avgMs; double overheadMs; };
    std::vector<Result> results;

    // Scène simple (un cube)
    SceneContext scene;
    scene.cameraPos = {0, 1.7f, 3};
    scene.viewMatrix = Mat4f::LookAt(scene.cameraPos, {0, 1.4f, 0}, {0, 1, 0});
    scene.projMatrix = Mat4f::Perspective(70.0f * 3.14159f / 180.0f, 1280.0f / 720.0f, 0.05f, 200.0f);
    Light sun; sun.type = LightType::DIRECTIONAL; sun.direction = {-0.4f, -1, -0.3f}; sun.color = {1, 0.95f, 0.85f}; sun.intensity = 3; sun.castShadow = true;
    scene.lights.push_back(sun);

    DrawCall3D draw;
    draw.meshHandle = MeshSystem::GetCube();
    draw.transform = Mat4f::Translate({0, 0.5f, -2}) * Mat4f::Scale({1, 1, 1});
    draw.tint = {1, 1, 1};
    draw.aabb.min = {-0.5f, 0, -2.5f}; draw.aabb.max = {0.5f, 1, -1.5f};

    double baseMs = 0;

    for (auto& test : tests) {
        Renderer3DConfig rcfg; rcfg.subsystems = test.mask; rcfg.vsync = false; rcfg.debug = false;
        Renderer3D renderer(window, rcfg);
        if (!renderer.IsValid()) continue;

        // Warmup
        for (int i = 0; i < 10; ++i) {
            renderer.BeginFrame(0xFF202030);
            renderer.SetScene(scene);
            renderer.Submit(draw);
            renderer.Execute();
            renderer.Present();
        }

        // Mesure
        auto start = high_resolution_clock::now();
        for (int i = 0; i < NUM_FRAMES; ++i) {
            renderer.BeginFrame(0xFF202030);
            renderer.SetScene(scene);
            renderer.Submit(draw);
            renderer.Execute();
            renderer.Present();
        }
        auto end = high_resolution_clock::now();
        
        double totalMs = duration<double, std::milli>(end - start).count();
        double avgMs = totalMs / NUM_FRAMES;
        
        double overhead = (results.empty() ? 0 : avgMs - baseMs);
        if (results.empty()) baseMs = avgMs;
        
        results.push_back({test.name, avgMs, overhead});
        
        mesuresFile << test.name << " : " << (long long)(avgMs * 1000) << " us\n";
        renderer.Shutdown();
    }

    // Classement (du plus cher au moins cher)
    std::sort(results.begin(), results.end(), [](const Result& a, const Result& b) {
        return a.avgMs > b.avgMs;
    });

    classementFile << "Classement du plus cher au moins cher (surcout par rapport a RENDER3D seul) :\n";
    for (auto& r : results) {
        classementFile << r.name << " : " << (long long)(r.avgMs * 1000) << " us/frame";
        if (r.overheadMs > 0) classementFile << " (surcout +" << (long long)(r.overheadMs * 1000) << " us)";
        classementFile << "\n";
    }

    // Choix pour un casque (budget 11ms = ~90 FPS)
    choixFile << "Je garde : RENDER3D (" << (long long)(baseMs * 1000) << " us/frame)\n";
    choixFile << "Pourquoi : Seul RENDER3D est necessaire pour afficher la salle. Les autres sous-systemes "
              << "(SHADOW, RAYTRACE, VISIBILITY) coutent du temps CPU/GPU sans apporter de visuel "
              << "pour cette scene simple. Sur un casque autonome (budget 11ms), il faut minimiser "
              << "la charge. RENDER3D seul tient largement dans le budget.\n";

    mesuresFile.close();
    classementFile.close();
    choixFile.close();

    Close();
    return 0;
}