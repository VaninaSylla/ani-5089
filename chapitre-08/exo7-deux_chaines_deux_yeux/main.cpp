#include <iostream>
#include <string>

struct Anneau {
    int suivant = 0;
    bool empruntee = false;
    int index = 0;
    int rendus = 0;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int images = 0;
    int n = 0;
    if (!(std::cin >> images >> n)) return 0;

    Anneau gauche;
    Anneau droit;
    int desync = 0;

    for (int tour = 1; tour <= n; ++tour) {
        std::string oeil;
        std::string commande;
        std::cin >> oeil >> commande;

        Anneau &anneau = (oeil == "G") ? gauche : droit;

        if (commande == "EMPRUNTE") {
            if (!anneau.empruntee && images > 0) {
                anneau.index = anneau.suivant;
                anneau.empruntee = true;
                std::cout << tour << ' ' << oeil << " EMPRUNTE "
                          << anneau.index << '\n';
            } else {
                std::cout << tour << ' ' << oeil << " EMPRUNTE REFUSE\n";
            }
        } else if (commande == "REND") {
            if (anneau.empruntee) {
                std::cout << tour << ' ' << oeil << " REND " << anneau.index
                          << '\n';
                anneau.empruntee = false;
                anneau.suivant = (anneau.suivant + 1) % images;
                ++anneau.rendus;
            } else {
                std::cout << tour << ' ' << oeil << " REND REFUSE\n";
            }
        }

        int difference = gauche.rendus - droit.rendus;
        if (difference < 0) difference = -difference;
        if (difference > 1 && desync == 0) desync = tour;
    }

    int decalage = gauche.rendus - droit.rendus;
    if (decalage < 0) decalage = -decalage;

    std::cout << "DECALAGE " << decalage << '\n';
    std::cout << "DESYNC " << desync << '\n';
    return 0;
}
