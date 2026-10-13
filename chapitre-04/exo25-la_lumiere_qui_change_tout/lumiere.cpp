// exo25 - La lumière qui change tout
// Fait varier direction, intensité, ombres du soleil et juge la lisibilité

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>

#include "win32_renderer.h"

using namespace nkentseu;
using namespace std::chrono;

void waitForKey() {
    std::cout << "Appuyez sur Entree pour continuer..." << std::endl;
    // Wait for Enter key using async key state (works without message pump)
    while (true) {
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
            break;
        }
        Sleep(50);
    }
    // Wait for key release
    while (GetAsyncKeyState(VK_RETURN) & 0x8000) {
        Sleep(10);
    }
    std::cout << "Continue..." << std::endl;
}

int main() {
    AppData appData; appData.appName = "exo25 - Lumiere";
    if (!Initialise(appData)) return 1;

    WindowConfig wcfg; wcfg.title = "exo25 - Lumiere"; wcfg.width = 1280; wcfg.height = 720;
    Window window(wcfg); 
    window.SetVisible(true);
    if (!window.IsValid()) {
        std::cerr << "ERROR: Window creation failed!" << std::endl;
        return 1;
    }
    std::cout << "Window created: " << window.GetWidth() << "x" << window.GetHeight() << std::endl;

    Renderer3DConfig rcfg; rcfg.subsystems = SS_RENDER3D | SS_SHADOW; rcfg.vsync = false; rcfg.debug = false;
    Renderer3D renderer(window, rcfg);
    if (!renderer.IsValid()) return 1;

    SceneContext scene;
    scene.cameraPos = {0.0f, 1.70f, 3.0f};
    scene.viewMatrix = Mat4f::LookAt(scene.cameraPos, {0.0f, 1.40f, 0.0f}, {0.0f, 1.0f, 0.0f});
    scene.projMatrix = Mat4f::Perspective(70.0f * 3.14159f / 180.0f, 1280.0f / 720.0f, 0.05f, 200.0f);

    Light sun;
    sun.type = LightType::DIRECTIONAL;
    sun.direction = {-0.4f, -1.0f, -0.3f};
    sun.color = {1.0f, 0.95f, 0.85f};
    sun.intensity = 3.0f;
    sun.castShadow = true;
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

    // --- 1. DEPART ---
    std::cout << "=== 1. DEPART (soleil standard) ===" << std::endl;
    bool running = true;
    int frameCount = 0;
    while (running) {
        PollEvents();
        if (!window.IsOpen()) { running = false; break; }
        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);
        drawRoom(renderer);
        renderer.Execute();
        renderer.Present();
        frameCount++;
        if (frameCount % 60 == 0) std::cout << "Frame " << frameCount << std::endl;
        // Check for Enter key to advance
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
            running = false;
        }
    }
    std::cout << "Total frames: " << frameCount << std::endl;
    // Wait for key release
    while (GetAsyncKeyState(VK_RETURN) & 0x8000) Sleep(10);

    // --- 2. DIRECTION CHANGÉE ---
    scene.lights[0].direction = {1.0f, -0.5f, 0.0f}; // Soleil rasant de côté
    running = true;
    frameCount = 0;
    std::cout << "=== 2. DIRECTION CHANGEE (soleil rasant) ===" << std::endl;
    while (running) {
        PollEvents();
        if (!window.IsOpen()) { running = false; break; }
        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);
        drawRoom(renderer);
        renderer.Execute();
        renderer.Present();
        frameCount++;
        if (frameCount % 60 == 0) std::cout << "Frame " << frameCount << std::endl;
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) running = false;
    }
    std::cout << "Total frames: " << frameCount << std::endl;
    while (GetAsyncKeyState(VK_RETURN) & 0x8000) Sleep(10);

    // --- 3. INTENSITÉ CHANGÉE ---
    scene.lights[0].direction = {-0.4f, -1.0f, -0.3f}; // Retour départ
    scene.lights[0].intensity = 0.5f; // Très faible
    running = true;
    frameCount = 0;
    std::cout << "=== 3. INTENSITE FAIBLE (0.5) ===" << std::endl;
    while (running) {
        PollEvents();
        if (!window.IsOpen()) { running = false; break; }
        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);
        drawRoom(renderer);
        renderer.Execute();
        renderer.Present();
        frameCount++;
        if (frameCount % 60 == 0) std::cout << "Frame " << frameCount << std::endl;
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) running = false;
    }
    std::cout << "Total frames: " << frameCount << std::endl;
    while (GetAsyncKeyState(VK_RETURN) & 0x8000) Sleep(10);

    // --- 4. OMBRES DÉSACTIVÉES ---
    scene.lights[0].intensity = 3.0f; // Retour départ
    scene.lights[0].castShadow = false;
    running = true;
    frameCount = 0;
    std::cout << "=== 4. SANS OMBRES ===" << std::endl;
    while (running) {
        PollEvents();
        if (!window.IsOpen()) { running = false; break; }
        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);
        drawRoom(renderer);
        renderer.Execute();
        renderer.Present();
        frameCount++;
        if (frameCount % 60 == 0) std::cout << "Frame " << frameCount << std::endl;
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) running = false;
    }
    std::cout << "Total frames: " << frameCount << std::endl;
    while (GetAsyncKeyState(VK_RETURN) & 0x8000) Sleep(10);

    // --- lisibilite.txt ---
    std::ofstream lisFile("lisibilite.txt");
    lisFile << "Direction :\n";
    lisFile << "  Valeur essayee : direction (1.0, -0.5, 0.0) - soleil rasant de cote\n";
    lisFile << "  Gain/perte en lisibilite : Les ombres longues reveleent la texture du sol et la hauteur des objets (table, pieds). "
            << "Les murs perpendiculaires au soleil sont bien eclaires, mais le mur oppose plonge dans l'ombre. "
            << "On perd en lisibilite globale car une grande partie de la piece est dans l'ombre portee, "
            << "mais on gagne en perception de relief sur les objets eclairés.\n\n";
    lisFile << "Intensite :\n";
    lisFile << "  Valeur essayee : intensite 0.5 (tres faible)\n";
    lisFile << "  Gain/perte en lisibilite : La scene devient sombre, les details dans les zones d'ombre disparaissent. "
            << "Les contours des objets sont moins nets, la perception de profondeur diminue. "
            << "L'oeil a du mal a separer les plans (mur/fond, table/sol). La lisibilite globale chute fortement.\n\n";
    lisFile << "Ombres :\n";
    lisFile << "  Valeur essayee : castShadow = false\n";
    lisFile << "  Gain/perte en lisibilite : Sans ombres, la table et les pieds flottent visuellement - "
            << "on ne voit plus le contact au sol. Les coins de la piece perdent en definition. "
            << "L'ancrage des objets au sol n'est plus perceptible, la piece parait plate. "
            << "Lisibilite des distances et volumes fortement degradee.\n";
    lisFile.close();

    renderer.Shutdown();
    Close();
    return 0;
}