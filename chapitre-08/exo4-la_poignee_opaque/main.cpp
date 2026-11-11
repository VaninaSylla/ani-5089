#include <iostream>
#include <string>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int images = 0;
    int n = 0;
    if (!(std::cin >> images >> n)) return 0;

    const int taille = images > 0 ? images : 0;
    std::vector<unsigned long long> valeurs(static_cast<std::size_t>(taille), 0ULL);
    std::vector<bool> posees(static_cast<std::size_t>(taille), false);
    int distincts = 0;

    for (int tour = 1; tour <= n; ++tour) {
        std::string commande;
        std::cin >> commande;

        if (commande == "POSE") {
            int index = 0;
            unsigned long long valeur = 0ULL;
            std::cin >> index >> valeur;

            if (index < 0 || index >= taille) {
                std::cout << "HORS ANNEAU\n";
            } else {
                if (!posees[static_cast<std::size_t>(index)]) {
                    posees[static_cast<std::size_t>(index)] = true;
                    ++distincts;
                }
                valeurs[static_cast<std::size_t>(index)] = valeur;
            }
        } else if (commande == "LIT") {
            int index = 0;
            std::cin >> index;

            if (index < 0 || index >= taille ||
                !posees[static_cast<std::size_t>(index)]) {
                std::cout << "VIDE\n";
            } else {
                std::cout << valeurs[static_cast<std::size_t>(index)] << '\n';
            }
        }
    }

    std::cout << "POSEES " << distincts << '\n';
    return 0;
}
