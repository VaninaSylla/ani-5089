#include <iostream>
#include <string>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int largeur = 0;
    int hauteur = 0;
    int n = 0;
    if (!(std::cin >> largeur >> hauteur >> n)) return 0;

    int bloquantes = 0;

    for (int i = 0; i < n; ++i) {
        std::string nom;
        int cible = 0;
        int taille = 0;
        int lc = 0;
        int hc = 0;
        std::cin >> nom >> cible >> taille >> lc >> hc;

        int utiliseeL = largeur;
        int utiliseeH = hauteur;
        std::string verdict;

        if (cible == 0) {
            verdict = "PAS RELIEE";
        } else if (taille == 1) {
            utiliseeL = lc;
            utiliseeH = hc;
            verdict = "OK";
        } else {
            if (utiliseeL != lc || utiliseeH != hc) {
                verdict = "BLOQUE";
                ++bloquantes;
            } else {
                verdict = "SANS EFFET";
            }
        }

        std::cout << nom << ' ' << utiliseeL << ' ' << utiliseeH << ' '
                  << verdict << '\n';
    }

    std::cout << "BLOQUANTES " << bloquantes << '\n';
    return 0;
}
