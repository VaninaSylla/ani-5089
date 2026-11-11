#include <iostream>
#include <string>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int vol = 0;
    int n = 0;
    if (!(std::cin >> vol >> n)) return 0;

    int enVol = 0;
    int bloque = 0;

    for (int tour = 1; tour <= n; ++tour) {
        std::string commande;
        std::cin >> commande;

        bool passe = true;

        if (commande == "SOUMET") {
            if (enVol >= vol) {
                passe = false;
                if (bloque == 0) bloque = tour;
            } else {
                ++enVol;
            }
        } else if (commande == "FINI") {
            if (enVol > 0) --enVol;
        }

        std::cout << tour << ' ' << commande << ' ' << enVol << ' '
                  << (passe ? "PASSE" : "ATTEND") << '\n';
    }

    std::cout << "BLOQUE " << bloque << '\n';
    return 0;
}
