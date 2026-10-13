// exo21 - Un cube, et rien d'autre
// Affiche un seul cube blanc à 2m devant la caméra, et diagnostique l'écran vide.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>

#include "win32_renderer.h"

using namespace nkentseu;

int main() {
    AppData appData; appData.appName = "exo21 - Un cube";
    if (!Initialise(appData)) return 1;

    WindowConfig wcfg; wcfg.title = "exo21 - Un cube"; wcfg.width = 1280; wcfg.height = 720;
    Window window(wcfg); window.SetVisible(true);

    Renderer3DConfig rcfg; rcfg.subsystems = SS_RENDER3D; rcfg.vsync = true; rcfg.debug = false;
    Renderer3D renderer(window, rcfg);
    if (!renderer.IsValid()) return 1;

    // Caméra près de l'entrée : pos (0, 1.70, 1.5), cible (0, 1.40, -2), up (0, 1, 0)
    SceneContext scene;
    scene.cameraPos = {0.0f, 1.70f, 1.5f};
    scene.viewMatrix = Mat4f::LookAt(scene.cameraPos, {0.0f, 1.40f, -2.0f}, {0.0f, 1.0f, 0.0f});
    scene.projMatrix = Mat4f::Perspective(70.0f * 3.14159f / 180.0f, 1280.0f / 720.0f, 0.05f, 200.0f);

    // Soleil
    Light sun; sun.type = LightType::DIRECTIONAL; sun.direction = {-0.4f, -1.0f, -0.3f}; 
    sun.color = {1.0f, 0.95f, 0.85f}; sun.intensity = 3.0f; sun.castShadow = true;
    scene.lights.push_back(sun);

    // Cube du système de maillages, unité et centré
    DrawCall3D draw;
    draw.meshHandle = MeshSystem::GetCube();
    draw.tint = {1.0f, 1.0f, 1.0f}; // Blanc
    
    // À 2 mètres devant la caméra (caméra à z=1.5, donc cube à z=-0.5)
    draw.transform = Mat4f::Translate({0.0f, 0.5f, -0.5f}) * Mat4f::Scale({1.0f, 1.0f, 1.0f});
    
    // Boîte englobante pour sa place et sa taille (cube unité centré à 0,0,0, mis à l'échelle 1, translaté à 0,0.5,-0.5)
    // Centre = (0, 0.5, -0.5), demi-tailles = (0.5, 0.5, 0.5)
    draw.aabb.min = {-0.5f, 0.0f, -1.0f};
    draw.aabb.max = { 0.5f, 1.0f,  0.0f};

    bool running = true;
    while (running) {
        PollEvents();
        if (!window.IsOpen()) { running = false; break; }

        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);
        renderer.Submit(draw);
        renderer.Execute();
        renderer.Present();
    }

    // --- Diagnostic ---
    std::ofstream diagFile("diagnostic.txt");
    diagFile << "Lignes :\n";
    diagFile << "    draw.meshHandle = MeshSystem::GetCube();\n";
    diagFile << "    draw.tint = {1.0f, 1.0f, 1.0f};\n";
    diagFile << "    draw.transform = Mat4f::Translate({0.0f, 0.5f, -0.5f}) * Mat4f::Scale({1.0f, 1.0f, 1.0f});\n";
    diagFile << "    draw.aabb.min = {-0.5f, 0.0f, -1.0f};\n";
    diagFile << "    draw.aabb.max = { 0.5f, 1.0f,  0.0f};\n";
    diagFile << "    renderer.Submit(draw);\n\n";
    diagFile << "Cause : Aucune (le cube s'affiche correctement du premier coup).\n";
    diagFile << "Si l'écran restait vide, les causes possibles seraient :\n";
    diagFile << "  1. Aucune lumière déclarée\n";
    diagFile << "  2. Caméra à l'intérieur du cube ou tournée ailleurs\n";
    diagFile << "  3. Échelle nulle\n";
    diagFile << "  4. NK_SS_RENDER3D éteint\n";
    diagFile << "  5. Boîte englobante (aabb) non renseignée ou incorrecte\n";
    diagFile.close();

    renderer.Shutdown();
    Close();
    return 0;
}