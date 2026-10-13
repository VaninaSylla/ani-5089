// exo8 - Votre première salle en cubes
// Dessine les quatre murs, le sol et le plafond d'une salle 4x4x2.5m

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>

#include "win32_renderer.h"

using namespace nkentseu;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int show) {
    // --- Initialiser le système ---
    AppData appData;
    appData.appName = "exo8 - Salle en cubes";
    if (!Initialise(appData)) {
        MessageBoxA(nullptr, "Echec initialisation", "Erreur", MB_ICONERROR);
        return 1;
    }

    // --- Configuration fenêtre ---
    WindowConfig wcfg;
    wcfg.title = "exo8 - Salle en cubes";
    wcfg.width = 1280;
    wcfg.height = 720;
    wcfg.resizable = true;
    
    Window window(wcfg);
    if (!window.IsValid()) {
        MessageBoxA(nullptr, "Echec creation fenetre", "Erreur", MB_ICONERROR);
        return 1;
    }
    window.SetVisible(true);

    // --- Créer le renderer 3D avec sous-systèmes nommés ---
    Renderer3DConfig rcfg;
    rcfg.subsystems = SS_RENDER3D;  // Seulement le rendu 3D, pas le défaut
    rcfg.vsync = true;
    rcfg.debug = false;

    Renderer3D renderer(window, rcfg);
    if (!renderer.IsValid()) {
        MessageBoxA(nullptr, "Echec creation renderer 3D", "Erreur", MB_ICONERROR);
        return 1;
    }

    // --- Afficher et sauvegarder le nom de l'interface graphique ---
    const char* apiName = renderer.GetApiName();
    std::cout << "Interface graphique: " << apiName << std::endl;
    
    std::ofstream backendFile("backend.txt");
    if (backendFile.is_open()) {
        backendFile << apiName << std::endl;
        backendFile.close();
    }

    // --- Préparer la scène ---
    SceneContext scene;
    scene.cameraPos = {0.0f, 1.70f, 3.0f};
    scene.viewMatrix = Mat4f::LookAt(
        scene.cameraPos,
        {0.0f, 1.40f, 0.0f},
        {0.0f, 1.0f, 0.0f}
    );
    scene.projMatrix = Mat4f::Perspective(
        70.0f * 3.1415926535f / 180.0f,
        1280.0f / 720.0f,
        0.05f,
        200.0f
    );

    // Lumière directionnelle (soleil)
    Light sun;
    sun.type = LightType::DIRECTIONAL;
    sun.direction = {-0.4f, -1.0f, -0.3f};
    sun.color = {1.0f, 0.95f, 0.85f};
    sun.intensity = 3.0f;
    sun.castShadow = true;
    scene.lights.push_back(sun);

    // --- Boucle de rendu ---
    bool running = true;
    
    while (running) {
        PollEvents();
        
        if (!window.IsOpen()) {
            running = false;
            break;
        }

        // 1. Préparer l'image
        renderer.BeginFrame(0xFF202030);

        // 2. Poser la scène (caméra + lumières)
        renderer.SetScene(scene);

        // 3. Soumettre les objets (salle 4x4x2.5, origine au centre du sol)
        DrawCall3D draw;
        draw.meshHandle = MeshSystem::GetCube();
        draw.tint = {0.72f, 0.70f, 0.66f};

        // --- Mur du fond (z = -2) ---
        draw.transform = Mat4f::Translate({0.0f, 1.25f, -2.0f}) 
                       * Mat4f::Scale({4.0f, 2.5f, 0.1f});
        draw.aabb.min = {-2.0f, 0.0f, -2.05f};
        draw.aabb.max = { 2.0f, 2.5f, -1.95f};
        renderer.Submit(draw);

        // --- Mur de l'entrée (z = +2) ---
        draw.transform = Mat4f::Translate({0.0f, 1.25f, 2.0f}) 
                       * Mat4f::Scale({4.0f, 2.5f, 0.1f});
        draw.aabb.min = {-2.0f, 0.0f, 1.95f};
        draw.aabb.max = { 2.0f, 2.5f, 2.05f};
        renderer.Submit(draw);

        // --- Mur de gauche (x = -2) ---
        draw.transform = Mat4f::Translate({-2.0f, 1.25f, 0.0f}) 
                       * Mat4f::Scale({0.1f, 2.5f, 4.0f});
        draw.aabb.min = {-2.05f, 0.0f, -2.0f};
        draw.aabb.max = {-1.95f, 2.5f,  2.0f};
        renderer.Submit(draw);

        // --- Mur de droite (x = +2) ---
        draw.transform = Mat4f::Translate({2.0f, 1.25f, 0.0f}) 
                       * Mat4f::Scale({0.1f, 2.5f, 4.0f});
        draw.aabb.min = {1.95f, 0.0f, -2.0f};
        draw.aabb.max = {2.05f, 2.5f,  2.0f};
        renderer.Submit(draw);

        // --- Plafond (y = 2.5) ---
        draw.transform = Mat4f::Translate({0.0f, 2.5f, 0.0f}) 
                       * Mat4f::Scale({4.0f, 0.1f, 4.0f});
        draw.tint = {0.85f, 0.85f, 0.85f};
        draw.aabb.min = {-2.0f, 2.45f, -2.0f};
        draw.aabb.max = { 2.0f, 2.55f,  2.0f};
        renderer.Submit(draw);

        // --- Sol (plan) ---
        draw.meshHandle = MeshSystem::GetPlane();
        draw.transform = Mat4f::Translate({0.0f, 0.0f, 0.0f}) 
                       * Mat4f::Scale({4.0f, 1.0f, 4.0f});
        draw.tint = {0.45f, 0.40f, 0.35f};
        draw.aabb.min = {-2.0f, -0.01f, -2.0f};
        draw.aabb.max = { 2.0f,  0.01f,  2.0f};
        renderer.Submit(draw);

        // 4. Exécuter les ordres
        renderer.Execute();

        // 5. Présenter
        renderer.Present();
    }

    renderer.Shutdown();
    Close();
    return 0;
}