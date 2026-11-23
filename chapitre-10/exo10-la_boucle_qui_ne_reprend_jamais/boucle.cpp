#include <iostream>

int main() {
    int frame = 0;
    bool shouldRender = true;

    while (frame < 20) {
        frame++;
        bool render = shouldRender;

        std::cout << "trame : " << frame << ", rend : " << render << ", debut : 1";

        if (render) {
            // rendu conditionnel
        }

        // Fin de trame hors du test de rendu.
        std::cout << ", fin : 1.\n";

        if (frame >= 6 && frame <= 10) {
            shouldRender = false;
        } else if (frame >= 11) {
            shouldRender = true;
        }
    }

    return 0;
}
