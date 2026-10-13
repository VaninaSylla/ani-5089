// exo23 - La table
// Plateau à 75cm du sol, 4 pieds. Calcul des positions.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>

#include "win32_renderer.h"

using namespace nkentseu;

int main() {
    AppData appData; appData.appName = "exo23 - La table";
    if (!Initialise(appData)) return 1;

    WindowConfig wcfg; wcfg.title = "exo23 - La table"; wcfg.width = 1280; wcfg.height = 720;
    Window window(wcfg); window.SetVisible(true);

    Renderer3DConfig rcfg; rcfg.subsystems = SS_RENDER3D; rcfg.vsync = true; rcfg.debug = false;
    Renderer3D renderer(window, rcfg);
    if (!renderer.IsValid()) return 1;

    // Scène
    SceneContext scene;
    scene.cameraPos = {0.0f, 1.70f, 3.0f};
    scene.viewMatrix = Mat4f::LookAt(scene.cameraPos, {0.60f, 1.0f, 0.70f}, {0.0f, 1.0f, 0.0f});
    scene.projMatrix = Mat4f::Perspective(70.0f * 3.14159f / 180.0f, 1280.0f / 720.0f, 0.05f, 200.0f);
    Light sun; sun.type = LightType::DIRECTIONAL; sun.direction = {-0.4f, -1.0f, -0.3f}; 
    sun.color = {1.0f, 0.95f, 0.85f}; sun.intensity = 3.0f; sun.castShadow = true;
    scene.lights.push_back(sun);

    DrawCall3D draw;
    draw.meshHandle = MeshSystem::GetCube();

    // --- CALCULS ---
    // Table : 1.20m long (x), 0.80m profond (z), centrée en x=0.60, z=0.70
    // Dessus plateau à 0.75m du sol. Plateau épais 5cm (0.05m).
    // Cube unitaire : centre à l'origine, va de -0.5 à +0.5
    
    // PLATEAU : échelle (1.20, 0.05, 0.80) -> arêtes 1.20, 0.05, 0.80, centre à l'origine
    // Dessus du plateau à y = 0.75 => centre du plateau à y = 0.75 - 0.025 = 0.725
    // Translation : (0.60, 0.725, 0.70)
    
    // PIEDS : section 6cm x 6cm (0.06 x 0.06), hauteur = 0.75 - 0.05 = 0.70m (du sol au dessous du plateau)
    // Centre des pieds à y = 0.70 / 2 = 0.35
    // Coins du plateau : x = 0.60 ± 0.60, z = 0.70 ± 0.40
    // Pieds au centre de chaque coin : x = ±0.60, z = ±0.40 (relatif au centre table 0.60, 0.70)
    // Donc positions absolues : (0.60±0.60, 0.35, 0.70±0.40) = (1.20, 0.35, 1.10), (0, 0.35, 1.10), (1.20, 0.35, 0.30), (0, 0.35, 0.30)

    bool running = true;
    while (running) {
        PollEvents();
        if (!window.IsOpen()) { running = false; break; }

        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);

        // Plateau
        draw.tint = {0.55f, 0.45f, 0.35f}; // Marron bois
        draw.transform = Mat4f::Translate({0.60f, 0.725f, 0.70f}) * Mat4f::Scale({1.20f, 0.05f, 0.80f});
        draw.aabb.min = {0.0f, 0.70f, 0.30f}; draw.aabb.max = {1.20f, 0.75f, 1.10f};
        renderer.Submit(draw);

        // 4 pieds
        draw.tint = {0.4f, 0.3f, 0.2f}; // Plus foncé
        draw.transform = Mat4f::Translate({1.20f, 0.35f, 1.10f}) * Mat4f::Scale({0.06f, 0.70f, 0.06f});
        draw.aabb.min = {1.17f, 0, 1.07f}; draw.aabb.max = {1.23f, 0.70f, 1.13f};
        renderer.Submit(draw);

        draw.transform = Mat4f::Translate({0.0f, 0.35f, 1.10f}) * Mat4f::Scale({0.06f, 0.70f, 0.06f});
        draw.aabb.min = {-0.03f, 0, 1.07f}; draw.aabb.max = {0.03f, 0.70f, 1.13f};
        renderer.Submit(draw);

        draw.transform = Mat4f::Translate({1.20f, 0.35f, 0.30f}) * Mat4f::Scale({0.06f, 0.70f, 0.06f});
        draw.aabb.min = {1.17f, 0, 0.27f}; draw.aabb.max = {1.23f, 0.70f, 0.33f};
        renderer.Submit(draw);

        draw.transform = Mat4f::Translate({0.0f, 0.35f, 0.30f}) * Mat4f::Scale({0.06f, 0.70f, 0.06f});
        draw.aabb.min = {-0.03f, 0, 0.27f}; draw.aabb.max = {0.03f, 0.70f, 0.33f};
        renderer.Submit(draw);

        renderer.Execute();
        renderer.Present();
    }

    // --- calcul.txt ---
    std::ofstream calcFile("calcul.txt");
    calcFile << "Plateau :\n";
    calcFile << "  y_centre = 0.75 - 0.05/2 = 0.75 - 0.025 = 0.725 m\n";
    calcFile << "  (dessus du plateau a 0.75m, plateau epais 0.05m, donc centre a 0.025m en dessous)\n\n";
    calcFile << "Pieds :\n";
    calcFile << "  hauteur = 0.75 - 0.05 = 0.70 m (du sol au dessous du plateau)\n";
    calcFile << "  y_centre = 0.70 / 2 = 0.35 m\n";
    calcFile << "  positions aux 4 coins : (±0.60, 0.35, ±0.40) relatifs au centre table (0.60, 0.70)\n";
    calcFile << "  soit : (1.20, 0.35, 1.10), (0.00, 0.35, 1.10), (1.20, 0.35, 0.30), (0.00, 0.35, 0.30)\n";
    calcFile.close();

    renderer.Shutdown();
    Close();
    return 0;
}