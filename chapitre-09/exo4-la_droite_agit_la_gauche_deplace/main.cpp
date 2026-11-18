#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

namespace {

bool EstConnu(const std::string &usage) {
    static const std::unordered_set<std::string> connus = {
        "SELECT",       "GRAB",         "MENU",          "MOVE",
        "AIM_POSE",     "GRIP_POSE",    "HAPTIC",        "SELECT_LEFT",
        "GRAB_LEFT",    "AIM_POSE_LEFT", "GRIP_POSE_LEFT", "HAPTIC_LEFT",
        "MOVE_RIGHT"};
    return connus.count(usage) != 0;
}

bool EstGauche(const std::string &usage) {
    return usage == "MENU" || usage == "MOVE" ||
           (usage.size() >= 5 && usage.compare(usage.size() - 5, 5, "_LEFT") == 0);
}

}

int main() {
    int n = 0;
    std::cin >> n;
    std::cin >> std::ws;

    int deuxMains = 0;
    for (int i = 0; i < n; ++i) {
        std::string ligne;
        std::getline(std::cin, ligne);
        std::istringstream flux(ligne);

        std::string nom;
        flux >> nom;

        int droite = 0;
        int gauche = 0;
        std::string usage;
        while (flux >> usage) {
            if (!EstConnu(usage)) {
                continue;
            }
            if (EstGauche(usage)) {
                ++gauche;
            } else {
                ++droite;
            }
        }

        const bool deux = droite > 0 && gauche > 0;
        if (deux) {
            ++deuxMains;
        }
        std::cout << nom << ' ' << droite << ' ' << gauche << ' '
                  << (deux ? "DEUX MAINS" : "UNE MAIN") << '\n';
    }

    std::cout << "DEUX MAINS " << deuxMains << '\n';
    return 0;
}
