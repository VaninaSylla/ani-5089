// exo9 - Le mauvais ordre, mesuré
// Dessine la même salle deux fois : une fois avec Translate * Scale (correct)
// et une fois avec Scale * Translate (incorrect), et mesure l'écart.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>
#include <cmath>

#include "win32_renderer.h"

using namespace nkentseu;

struct RoomWall {
    const char* name;
    Vec3f scale;      // Scale du mur
    Vec3f translate;  // Position du centre
    Vec3f tint;       // Couleur
};

int main() {
    // --- Initialiser ---
    AppData appData; appData.appName = "exo9 - Mauvais ordre";
    if (!Initialise(appData)) return 1;

    // Fenêtre pour le bon ordre
    WindowConfig wcfg1; wcfg1.title = "exo9 - Bon ordre (Translate * Scale)"; wcfg1.width = 1280; wcfg1.height = 720;
    Window windowBon(wcfg1); windowBon.SetVisible(true);

    // Fenêtre pour le mauvais ordre
    WindowConfig wcfg2; wcfg2.title = "exo9 - Mauvais ordre (Scale * Translate)"; wcfg2.width = 1280; wcfg2.height = 720;
    Window windowMauvais(wcfg2); windowMauvais.SetVisible(true);

    Renderer3DConfig rcfg; rcfg.subsystems = SS_RENDER3D; rcfg.vsync = true; rcfg.debug = false;
    Renderer3D rendererBon(windowBon, rcfg);
    Renderer3D rendererMauvais(windowMauvais, rcfg);

    if (!rendererBon.IsValid() || !rendererMauvais.IsValid()) return 1;

    // Scène identique pour les deux
    SceneContext scene;
    scene.cameraPos = {0.0f, 1.70f, 3.0f};
    scene.viewMatrix = Mat4f::LookAt(scene.cameraPos, {0.0f, 1.40f, 0.0f}, {0.0f, 1.0f, 0.0f});
    scene.projMatrix = Mat4f::Perspective(70.0f * 3.1415926535f / 180.0f, 1280.0f / 720.0f, 0.05f, 200.0f);
    Light sun; sun.type = LightType::DIRECTIONAL; sun.direction = {-0.4f, -1.0f, -0.3f}; sun.color = {1.0f, 0.95f, 0.85f}; sun.intensity = 3.0f; sun.castShadow = true;
    scene.lights.push_back(sun);

    // Définition des murs de la salle 4x4x2.5 (origine au centre du sol)
    RoomWall walls[] = {
        {"Fond",      {4.0f, 2.5f, 0.1f}, {0.0f, 1.25f, -2.0f}, {0.72f, 0.70f, 0.66f}},
        {"Entree",    {4.0f, 2.5f, 0.1f}, {0.0f, 1.25f,  2.0f}, {0.72f, 0.70f, 0.66f}},
        {"Gauche",    {0.1f, 2.5f, 4.0f}, {-2.0f, 1.25f, 0.0f}, {0.72f, 0.70f, 0.66f}},
        {"Droite",    {0.1f, 2.5f, 4.0f}, { 2.0f, 1.25f, 0.0f}, {0.72f, 0.70f, 0.66f}},
        {"Plafond",   {4.0f, 0.1f, 4.0f}, {0.0f, 2.5f,  0.0f}, {0.85f, 0.85f, 0.85f}},
        {"Sol",       {4.0f, 1.0f, 4.0f}, {0.0f, 0.0f,  0.0f}, {0.45f, 0.40f, 0.35f}},
    };

    // --- Calculer les coordonnées du centre du mur du fond pour les deux ordres ---
    // Mur du fond : scale(4, 2.5, 0.1), translate(0, 1.25, -2)
    // Cube unitaire : centre à l'origine, va de -0.5 à +0.5
    
    // ORDRE CORRECT : Translate * Scale
    // D'abord Scale : cube devient 4x2.5x0.1 centré à l'origine (y de -1.25 à +1.25)
    // Ensuite Translate(0, 1.25, -2) : centre va à (0, 1.25, -2), bas du mur à y=0
    Vec3f centreBon = {0.0f, 1.25f, -2.0f};
    
    // ORDRE INCORRECT : Scale * Translate
    // D'abord Translate(0, 1.25, -2) : centre du cube unitaire va à (0, 1.25, -2)
    // Ensuite Scale(4, 2.5, 0.1) : le centre est AUSSI mis à l'échelle -> (0*4, 1.25*2.5, -2*0.1) = (0, 3.125, -0.2)
    Vec3f centreMauvais = {0.0f * 4.0f, 1.25f * 2.5f, -2.0f * 0.1f}; // = {0, 3.125, -0.2}

    // Écrire coordonnees.txt
    std::ofstream coordFile("coordonnees.txt");
    coordFile << "bon : " << centreBon.x << " " << centreBon.y << " " << centreBon.z << "\n";
    coordFile << "mauvais : " << centreMauvais.x << " " << centreMauvais.y << " " << centreMauvais.z << "\n";
    coordFile.close();

    // --- Écrire reponse.txt ---
    std::ofstream repFile("reponse.txt");
    repFile << "Facteur : 2.5\n";
    repFile << "Pourquoi : L'ordre Scale * Translate applique l'echelle AUSSI a la translation. "
            << "Le centre du mur qui devrait etre a y=1.25 (demi-hauteur 2.5/2) se retrouve a "
            << "y=1.25*2.5=3.125. L'echelle 'attrape' la translation et la multiplie. "
            << "C'est pourquoi la salle parait enorme : tous les objets sont echelles par rapport a l'origine "
            << "au lieu d'etre places puis echelles localement.\n";
    repFile.close();

    // --- Boucle de rendu pour les deux fenêtres ---
    bool running = true;
    while (running) {
        PollEvents();
        
        if (!windowBon.IsOpen() || !windowMauvais.IsOpen()) {
            running = false; break;
        }

        // --- FENÊTRE BON ORDRE ---
        rendererBon.BeginFrame(0xFF202030);
        rendererBon.SetScene(scene);
        
        DrawCall3D draw;
        draw.meshHandle = MeshSystem::GetCube();
        
        for (auto& w : walls) {
            draw.tint = w.tint;
            // ORDRE CORRECT : Translate * Scale
            draw.transform = Mat4f::Translate(w.translate) * Mat4f::Scale(w.scale);
            
            // AABB pour le bon ordre
            Vec3f half = {w.scale.x * 0.5f, w.scale.y * 0.5f, w.scale.z * 0.5f};
            draw.aabb.min = {w.translate.x - half.x, w.translate.y - half.y, w.translate.z - half.z};
            draw.aabb.max = {w.translate.x + half.x, w.translate.y + half.y, w.translate.z + half.z};
            rendererBon.Submit(draw);
        }
        
        rendererBon.Execute();
        rendererBon.Present();

        // --- FENÊTRE MAUVAIS ORDRE ---
        rendererMauvais.BeginFrame(0xFF202030);
        rendererMauvais.SetScene(scene);
        
        for (auto& w : walls) {
            draw.tint = w.tint;
            // ORDRE INCORRECT : Scale * Translate
            draw.transform = Mat4f::Scale(w.scale) * Mat4f::Translate(w.translate);
            
            // AABB pour le mauvais ordre (centre mis à l'échelle)
            Vec3f scaledCenter = {w.translate.x * w.scale.x, w.translate.y * w.scale.y, w.translate.z * w.scale.z};
            Vec3f half = {w.scale.x * 0.5f, w.scale.y * 0.5f, w.scale.z * 0.5f};
            draw.aabb.min = {scaledCenter.x - half.x, scaledCenter.y - half.y, scaledCenter.z - half.z};
            draw.aabb.max = {scaledCenter.x + half.x, scaledCenter.y + half.y, scaledCenter.z + half.z};
            rendererMauvais.Submit(draw);
        }
        
        rendererMauvais.Execute();
        rendererMauvais.Present();
    }

    rendererBon.Shutdown();
    rendererMauvais.Shutdown();
    Close();
    return 0;
}