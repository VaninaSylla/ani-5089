#include <iostream>
#include <string>
#include <unordered_map>

int main() {
    int a = 0;
    std::cin >> a;

    std::unordered_map<std::string, bool> estSortie;
    int entrees = 0;
    int sorties = 0;

    for (int i = 0; i < a; ++i) {
        std::string nom;
        std::string type;
        std::cin >> nom >> type;

        const bool sortie = type == "HAPTIC";
        estSortie[nom] = sortie;
        if (sortie) {
            ++sorties;
        } else {
            ++entrees;
        }
    }

    int o = 0;
    std::cin >> o;
    for (int i = 0; i < o; ++i) {
        std::string nom;
        std::string operation;
        std::cin >> nom >> operation;

        const auto it = estSortie.find(nom);
        if (it == estSortie.end()) {
            continue;
        }
        if (operation == "LIT" && it->second) {
            std::cout << nom << " LIT UNE SORTIE\n";
        } else if (operation == "ECRIT" && !it->second) {
            std::cout << nom << " ECRIT UNE ENTREE\n";
        }
    }

    std::cout << "ENTREES " << entrees << '\n';
    std::cout << "SORTIES " << sorties << '\n';
    return 0;
}
