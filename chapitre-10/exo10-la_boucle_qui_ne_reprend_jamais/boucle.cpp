#include <iostream>

int main() {
    int frame = 0;
    bool shouldRender = true;

    while (frame < 20) {
        frame++;
        bool render = shouldRender;

        // FAUTE : fin de trame appelee seulement si on rend.
        bool fin = render;

        std::cout << "trame : " << frame << ", rend : " << render
                  << ", debut : 1, fin : " << fin << ".\n";

        if (frame >= 6 && frame <= 10) {
            shouldRender = false;
        } else if (frame >= 11) {
            shouldRender = true;
        }
    }

    return 0;
}
