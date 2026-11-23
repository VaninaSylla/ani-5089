#include <iostream>
#include <string>
#include "couches.h"

int main() {
    int images = 3;
    int index = 0;

    for (int image = 1; image <= 6; image++) {
        bool dessiner = image != 3 && image != 4;
        int couches = dessiner ? 1 : 0;

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
    }

    return 0;
}
