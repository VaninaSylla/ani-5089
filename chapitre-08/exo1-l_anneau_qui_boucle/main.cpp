#include <iostream>
#include <string>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int images = 0;
    int n = 0;
    if (!(std::cin >> images >> n)) return 0;

    int suivant = 0;
    bool empruntee = false;
    int indexEmprunte = 0;
    int prets = 0;

    for (int tour = 1; tour <= n; ++tour) {
        std::string commande;
        std::cin >> commande;

        if (commande == "EMPRUNTE") {
            if (!empruntee && images > 0) {
                indexEmprunte = suivant;
                empruntee = true;
                ++prets;
                std::cout << tour << " EMPRUNTE " << indexEmprunte << '\n';
            } else {
                std::cout << tour << " EMPRUNTE REFUSE\n";
            }
        } else if (commande == "REND") {
            if (empruntee) {
                std::cout << tour << " REND " << indexEmprunte << '\n';
                empruntee = false;
                suivant = (suivant + 1) % images;
            } else {
                std::cout << tour << " REND REFUSE\n";
            }
        }
    }

    std::cout << "PRETS " << prets << '\n';
    return 0;
}
