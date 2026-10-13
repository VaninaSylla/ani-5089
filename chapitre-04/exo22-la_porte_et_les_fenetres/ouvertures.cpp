// exo22 - La porte et les fenêtres
// Ajoute la porte et deux fenêtres à la salle, règle le clignotement (z-fighting)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>

#include "win32_renderer.h"

using namespace nkentseu;

int main() {
    AppData appData; appData.appName = "exo22 - Porte et fenetres";
    if (!Initialise(appData)) return 1;

    WindowConfig wcfg; wcfg.title = "exo22 - Salle avec ouvertures"; wcfg.width = 1280; wcfg.height = 720;
    Window window(wcfg); window.SetVisible(true);

    Renderer3DConfig rcfg; rcfg.subsystems = SS_RENDER3D; rcfg.vsync = true; rcfg.debug = false;
    Renderer3D renderer(window, rcfg);
    if (!renderer.IsValid()) return 1;

    // Scène : salle 4x4x2.5, origine centre du sol
    SceneContext scene;
    scene.cameraPos = {0.0f, 1.70f, 3.0f};
    scene.viewMatrix = Mat4f::LookAt(scene.cameraPos, {0.0f, 1.40f, 0.0f}, {0.0f, 1.0f, 0.0f});
    scene.projMatrix = Mat4f::Perspective(70.0f * 3.14159f / 180.0f, 1280.0f / 720.0f, 0.05f, 200.0f);
    Light sun; sun.type = LightType::DIRECTIONAL; sun.direction = {-0.4f, -1.0f, -0.3f}; 
    sun.color = {1.0f, 0.95f, 0.85f}; sun.intensity = 3.0f; sun.castShadow = true;
    scene.lights.push_back(sun);

    DrawCall3D draw;
    draw.meshHandle = MeshSystem::GetCube();
    
    // Couleur mur
    Vec3f wallColor = {0.72f, 0.70f, 0.66f};
    // Couleur porte/fenêtres (différente)
    Vec3f openingColor = {0.4f, 0.3f, 0.2f}; // Marron foncé

    bool running = true;
    while (running) {
        PollEvents();
        if (!window.IsOpen()) { running = false; break; }

        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);

        // --- MURS (comme exo8) ---
        draw.tint = wallColor;
        
        // Mur fond (z = -2)
        draw.transform = Mat4f::Translate({0, 1.25f, -2}) * Mat4f::Scale({4, 2.5f, 0.1f});
        draw.aabb.min = {-2, 0, -2.05f}; draw.aabb.max = {2, 2.5f, -1.95f};
        renderer.Submit(draw);
        
        // Mur entrée (z = +2)
        draw.transform = Mat4f::Translate({0, 1.25f, 2}) * Mat4f::Scale({4, 2.5f, 0.1f});
        draw.aabb.min = {-2, 0, 1.95f}; draw.aabb.max = {2, 2.5f, 2.05f};
        renderer.Submit(draw);
        
        // Mur gauche (x = -2)
        draw.transform = Mat4f::Translate({-2, 1.25f, 0}) * Mat4f::Scale({0.1f, 2.5f, 4});
        draw.aabb.min = {-2.05f, 0, -2}; draw.aabb.max = {-1.95f, 2.5f, 2};
        renderer.Submit(draw);
        
        // Mur droite (x = +2)
        draw.transform = Mat4f::Translate({2, 1.25f, 0}) * Mat4f::Scale({0.1f, 2.5f, 4});
        draw.aabb.min = {1.95f, 0, -2}; draw.aabb.max = {2.05f, 2.5f, 2};
        renderer.Submit(draw);
        
        // Plafond
        draw.tint = {0.85f, 0.85f, 0.85f};
        draw.transform = Mat4f::Translate({0, 2.5f, 0}) * Mat4f::Scale({4, 0.1f, 4});
        draw.aabb.min = {-2, 2.45f, -2}; draw.aabb.max = {2, 2.55f, 2};
        renderer.Submit(draw);
        
        // Sol (plan)
        draw.meshHandle = MeshSystem::GetPlane();
        draw.tint = {0.45f, 0.40f, 0.35f};
        draw.transform = Mat4f::Translate({0, 0, 0}) * Mat4f::Scale({4, 1, 4});
        draw.aabb.min = {-2, -0.01f, -2}; draw.aabb.max = {2, 0.01f, 2};
        renderer.Submit(draw);
        draw.meshHandle = MeshSystem::GetCube();

        // --- PORTE : 0.90m large, 2.00m haut, centre en x=-0.70, sur mur fond (z=-2) ---
        // Face intérieure du mur fond : z = -2 + 0.05 = -1.95 (mur épais 0.1, centre à -2)
        // Porte épaisse 2cm = 0.02m, face avant dans le plan z = -1.95
        // Donc centre porte à z = -1.95 + 0.01 = -1.94
        draw.tint = openingColor;
        draw.transform = Mat4f::Translate({-0.70f, 1.0f, -1.94f}) * Mat4f::Scale({0.90f, 2.0f, 0.02f});
        draw.aabb.min = {-1.15f, 0, -1.95f}; draw.aabb.max = {-0.25f, 2.0f, -1.93f};
        renderer.Submit(draw);

        // --- FENÊTRE 1 : mur gauche (x=-2), centre z=-0.90, centre y=1.40 ---
        // Face intérieure mur gauche : x = -2 + 0.05 = -1.95
        // Fenêtre 1.20m large (en z), 1.00m haut, épaisse 2cm
        // Centre x = -1.95 + 0.01 = -1.94
        draw.transform = Mat4f::Translate({-1.94f, 1.40f, -0.90f}) * Mat4f::Scale({0.02f, 1.0f, 1.20f});
        draw.aabb.min = {-1.95f, 0.9f, -1.5f}; draw.aabb.max = {-1.93f, 1.9f, -0.3f};
        renderer.Submit(draw);

        // --- FENÊTRE 2 : mur gauche (x=-2), centre z=+0.90, centre y=1.40 ---
        draw.transform = Mat4f::Translate({-1.94f, 1.40f, 0.90f}) * Mat4f::Scale({0.02f, 1.0f, 1.20f});
        draw.aabb.min = {-1.95f, 0.9f, 0.3f}; draw.aabb.max = {-1.93f, 1.9f, 1.5f};
        renderer.Submit(draw);

        // --- AVANCER LES PANNEAUX VERS L'INTÉRIEUR pour éviter le z-fighting ---
        // On les avance de 1cm (0.01m) vers l'intérieur
        // Porte : z = -1.94 - 0.01 = -1.93 (vers l'intérieur = +z car mur fond à z=-2)
        draw.transform = Mat4f::Translate({-0.70f, 1.0f, -1.93f}) * Mat4f::Scale({0.90f, 2.0f, 0.02f});
        draw.aabb.min = {-1.15f, 0, -1.94f}; draw.aabb.max = {-0.25f, 2.0f, -1.92f};
        renderer.Submit(draw);

        // Fenêtre 1 : x = -1.94 + 0.01 = -1.93 (vers l'intérieur = +x)
        draw.transform = Mat4f::Translate({-1.93f, 1.40f, -0.90f}) * Mat4f::Scale({0.02f, 1.0f, 1.20f});
        draw.aabb.min = {-1.94f, 0.9f, -1.5f}; draw.aabb.max = {-1.92f, 1.9f, -0.3f};
        renderer.Submit(draw);

        // Fenêtre 2 : x = -1.93
        draw.transform = Mat4f::Translate({-1.93f, 1.40f, 0.90f}) * Mat4f::Scale({0.02f, 1.0f, 1.20f});
        draw.aabb.min = {-1.94f, 0.9f, 0.3f}; draw.aabb.max = {-1.92f, 1.9f, 1.5f};
        renderer.Submit(draw);

        renderer.Execute();
        renderer.Present();
    }

    // --- Clignotement.txt ---
    std::ofstream cligFile("clignotement.txt");
    cligFile << "Constat : Quand la porte et les fenetres sont placees exactement dans le plan de la face interieure des murs, on voit un clignotement (z-fighting) : les pixels du mur et de l'ouverture se disputent la meme profondeur, creant un scintillement aleatoire.\n\n";
    cligFile << "Nom : Z-fighting (conflit de profondeur). Cela se produit quand deux surfaces coplanaires (meme plan) sont rendues : le buffer de profondeur ne peut pas trancher definitivement laquelle est devant, le resultat alterne entre les deux a chaque frame.\n\n";
    cligFile << "Reglage : 0.01 metres (1 cm). On avance les panneaux de 1 cm vers l'interieur de la piece. Ni moins (0.005m laisserait encore du z-fighting sur certaines cartes), ni plus (inutile de les enfoncer davantage dans le mur, 1cm suffit pour separer les plans dans le buffer de profondeur).\n";
    cligFile.close();

    renderer.Shutdown();
    Close();
    return 0;
}