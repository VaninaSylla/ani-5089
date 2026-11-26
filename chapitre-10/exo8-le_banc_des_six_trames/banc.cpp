#include <iostream>
#include <string>
#include "couches.h"

// Une couche portera l'espace et une vue par oeil.
void finDeTrame() {
    // Toujours appelee, meme quand rien n'est rendu.
}

int main() {
    int images = 3;
    int index = 0;

    for (int image = 1; image <= 6; image++) {
        bool dessiner = image != 3 && image != 4;
        Couche couche;
        couche.espace = "STAGE";
        couche.gauche.presente = false;
        couche.droite.presente = false;
        int couches = 0;

        if (dessiner) {
            couche.gauche.presente = true;
            couche.droite.presente = true;
            couches = 1;
        }

        std::cout << image << " " << (dessiner ? "OUI" : "NON") << " " << couches << " ";

        if (dessiner) {
            std::cout << index << '\n';
            index++;
            if (index >= images) {
                index = 0;
            }
        } else {
            std::cout << "-\n";
        }

        finDeTrame();
    }

    return 0;
}
