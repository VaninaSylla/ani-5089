#include <iostream>

int main() {
    int n = 0;
    std::cin >> n;

    int precedent = 0;
    int ballesFront = 0;
    int ballesNiveau = 0;
    int tenuNiveau = 0;
    int tenuFront = 0;

    for (int image = 0; image < n; ++image) {
        int presse = 0;
        std::cin >> presse;

        const int front = presse && presse != precedent ? 1 : 0;
        if (front) {
            ++ballesFront;
            ++tenuFront;
        }
        if (presse) {
            ++ballesNiveau;
            ++tenuNiveau;
        }
        precedent = presse;
    }

    std::cout << "BALLES FRONT " << ballesFront << '\n';
    std::cout << "BALLES NIVEAU " << ballesNiveau << '\n';
    std::cout << "TENU NIVEAU " << tenuNiveau << '\n';
    std::cout << "TENU FRONT " << tenuFront << '\n';
    return 0;
}
