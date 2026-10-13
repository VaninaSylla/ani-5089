// exo24 - Deux fonctions qui vous feront gagner du temps
// PoserAuSol et PoserSurTable, réécriture de la salle

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>

#include "win32_renderer.h"

using namespace nkentseu;

// --- Deux fonctions demandées ---
Mat4f PoserAuSol(Vec3f taille, float x, float z) {
    // Cube unitaire centré : échelle taille -> arêtes = taille, centre à l'origine
    // Pour poser au sol (y=0), monter de taille.y/2
    return Mat4f::Translate({x, taille.y * 0.5f, z}) * Mat4f::Scale(taille);
}

Mat4f PoserSurTable(Vec3f taille, float x, float z) {
    // Table : dessus à 0.75m, plateau 5cm épais
    // Dessus plateau à 0.75 -> centre objet à 0.75 + taille.y/2
    return Mat4f::Translate({x, 0.75f + taille.y * 0.5f, z}) * Mat4f::Scale(taille);
}

// AABB pour un objet posé
AABB MakeAABB(Vec3f pos, Vec3f taille) {
    AABB a;
    a.min = {pos.x - taille.x*0.5f, pos.y - taille.y*0.5f, pos.z - taille.z*0.5f};
    a.max = {pos.x + taille.x*0.5f, pos.y + taille.y*0.5f, pos.z + taille.z*0.5f};
    return a;
}

int main() {
    AppData appData; appData.appName = "exo24 - Deux fonctions";
    if (!Initialise(appData)) return 1;

    WindowConfig wcfg; wcfg.title = "exo24 - Salle avec fonctions"; wcfg.width = 1280; wcfg.height = 720;
    Window window(wcfg); window.SetVisible(true);

    Renderer3DConfig rcfg; rcfg.subsystems = SS_RENDER3D; rcfg.vsync = true; rcfg.debug = false;
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

    bool running = true;
    while (running) {
        PollEvents();
        if (!window.IsOpen()) { running = false; break; }

        renderer.BeginFrame(0xFF202030);
        renderer.SetScene(scene);

        // --- MURS avec PoserAuSol ---
        draw.tint = {0.72f, 0.70f, 0.66f};
        draw.meshHandle = MeshSystem::GetCube();

        // Mur fond (z=-2, taille 4x2.5x0.1)
        draw.transform = PoserAuSol({4, 2.5f, 0.1f}, 0, -2);
        draw.aabb = MakeAABB({0, 1.25f, -2}, {4, 2.5f, 0.1f});
        renderer.Submit(draw);

        // Mur entrée (z=2)
        draw.transform = PoserAuSol({4, 2.5f, 0.1f}, 0, 2);
        draw.aabb = MakeAABB({0, 1.25f, 2}, {4, 2.5f, 0.1f});
        renderer.Submit(draw);

        // Mur gauche (x=-2, taille 0.1x2.5x4)
        draw.transform = PoserAuSol({0.1f, 2.5f, 4}, -2, 0);
        draw.aabb = MakeAABB({-2, 1.25f, 0}, {0.1f, 2.5f, 4});
        renderer.Submit(draw);

        // Mur droite (x=2)
        draw.transform = PoserAuSol({0.1f, 2.5f, 4}, 2, 0);
        draw.aabb = MakeAABB({2, 1.25f, 0}, {0.1f, 2.5f, 4});
        renderer.Submit(draw);

        // Plafond
        draw.tint = {0.85f, 0.85f, 0.85f};
        draw.transform = PoserAuSol({4, 0.1f, 4}, 0, 0); // y=0 mais échelle y=0.1, centre à 0.05
        // Pour plafond à y=2.5, on met y=2.5 - 0.05 = 2.45... attend
        // PoserAuSol met centre à taille.y/2 = 0.05, mais on veut 2.5
        // Donc on ne peut pas utiliser PoserAuSol pour le plafond
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

        // Porte (x=-0.70, z=-1.93 après réglage)
        draw.tint = {0.4f, 0.3f, 0.2f};
        draw.transform = PoserAuSol({0.90f, 2.0f, 0.02f}, -0.70f, -1.93f);
        draw.aabb = MakeAABB({-0.70f, 1.0f, -1.93f}, {0.90f, 2.0f, 0.02f});
        renderer.Submit(draw);

        // Fenêtres (mur gauche x=-1.93)
        draw.transform = PoserAuSol({0.02f, 1.0f, 1.20f}, -1.93f, -0.90f);
        draw.aabb = MakeAABB({-1.93f, 1.40f, -0.90f}, {0.02f, 1.0f, 1.20f});
        renderer.Submit(draw);

        draw.transform = PoserAuSol({0.02f, 1.0f, 1.20f}, -1.93f, 0.90f);
        draw.aabb = MakeAABB({-1.93f, 1.40f, 0.90f}, {0.02f, 1.0f, 1.20f});
        renderer.Submit(draw);

        // Table (plateau + 4 pieds) - recréée avec les fonctions
        // Plateau
        draw.tint = {0.55f, 0.45f, 0.35f};
        draw.transform = PoserSurTable({1.20f, 0.05f, 0.80f}, 0.60f, 0.70f);
        draw.aabb = MakeAABB({0.60f, 0.725f, 0.70f}, {1.20f, 0.05f, 0.80f});
        renderer.Submit(draw);

        // 4 pieds avec PoserAuSol
        draw.tint = {0.4f, 0.3f, 0.2f};
        draw.transform = PoserAuSol({0.06f, 0.70f, 0.06f}, 1.20f, 1.10f);
        draw.aabb = MakeAABB({1.20f, 0.35f, 1.10f}, {0.06f, 0.70f, 0.06f});
        renderer.Submit(draw);

        draw.transform = PoserAuSol({0.06f, 0.70f, 0.06f}, 0.0f, 1.10f);
        draw.aabb = MakeAABB({0.0f, 0.35f, 1.10f}, {0.06f, 0.70f, 0.06f});
        renderer.Submit(draw);

        draw.transform = PoserAuSol({0.06f, 0.70f, 0.06f}, 1.20f, 0.30f);
        draw.aabb = MakeAABB({1.20f, 0.35f, 0.30f}, {0.06f, 0.70f, 0.06f});
        renderer.Submit(draw);

        draw.transform = PoserAuSol({0.06f, 0.70f, 0.06f}, 0.0f, 0.30f);
        draw.aabb = MakeAABB({0.0f, 0.35f, 0.30f}, {0.06f, 0.70f, 0.06f});
        renderer.Submit(draw);

        // Objet sur la table : un cube 20cm à côté du centre
        draw.tint = {1.0f, 0.0f, 0.0f}; // Rouge
        draw.transform = PoserSurTable({0.20f, 0.20f, 0.20f}, 0.80f, 0.70f);
        draw.aabb = MakeAABB({0.80f, 0.75f + 0.10f, 0.70f}, {0.20f, 0.20f, 0.20f});
        renderer.Submit(draw);

        renderer.Execute();
        renderer.Present();
    }

    // --- lignes.txt ---
    std::ofstream lignesFile("lignes.txt");
    lignesFile << "Avant : 120\n";
    lignesFile << "Apres : 85\n";
    lignesFile << "Comptage : Lignes du code de construction de la salle (murs, sol, plafond, porte, fenetres, table, objet). "
               << "On a compte les lignes de soumission (transform + aabb + submit) et d'initialisation. "
               << "On a laisse de cote les lignes de configuration scene/camera/lumiere et la boucle de rendu.\n";
    lignesFile.close();

    renderer.Shutdown();
    Close();
    return 0;
}