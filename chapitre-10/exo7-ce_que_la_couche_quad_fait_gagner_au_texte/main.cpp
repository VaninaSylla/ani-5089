#include <iostream>
#include <string>

int main() {
    int rendu = 0;
    int deformation = 0;
    int reprojection = 0;
    int seuil = 0;
    std::cin >> rendu >> deformation >> reprojection >> seuil;

    int n = 0;
    std::cin >> n;
    int gains = 0;
    int perdus = 0;

    for (int i = 0; i < n; i++) {
        std::string nom;
        int pixels = 0;
        std::cin >> nom >> pixels;

        int scene = pixels;
        scene = scene * rendu / 1000;
        scene = scene * deformation / 1000;
        scene = scene * reprojection / 1000;

        int quad = pixels;
        const char *sceneVerdict = scene >= seuil ? "LISIBLE" : "FLOU";
        const char *quadVerdict = quad >= seuil ? "LISIBLE" : "FLOU";

        if (sceneVerdict == std::string("FLOU") && quadVerdict == std::string("LISIBLE")) {
            gains++;
        }
        if (sceneVerdict == std::string("LISIBLE") && quadVerdict == std::string("FLOU")) {
            perdus++;
        }

        std::cout << nom << " " << scene << " " << quad << " " << sceneVerdict << " " << quadVerdict << '\n';
    }

    std::cout << "GAGNES " << gains << '\n';
    std::cout << "PERDUS " << perdus << '\n';
    return 0;
}
