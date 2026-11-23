#include <iostream>
#include <string>
#include <unordered_map>

int main() {
    int n;
    std::cin >> n;

    std::unordered_map<std::string, std::string> causes = {
        {"BLOQUE_AU_WAIT", "FIN DE TRAME"},
        {"PAS_DE_REPRISE", "FIN SANS RENDU"},
        {"OEIL_NOIR", "CHAINE MANQUANTE"},
        {"TREMBLEMENT", "MAUVAIS INSTANT"},
        {"CORRECTION_DE_TRAVERS", "POSE FRAICHE"},
        {"SCENE_DECALEE", "MAUVAIS ESPACE"}
    };

    int diagnostiques = 0;
    int inconnus = 0;

    for (int i = 0; i < n; i++) {
        std::string symptome;
        std::cin >> symptome;

        auto it = causes.find(symptome);
        if (it == causes.end()) {
            std::cout << symptome << " INCONNU\n";
            inconnus++;
        } else {
            std::cout << symptome << " " << it->second << '\n';
            diagnostiques++;
        }
    }

    std::cout << "DIAGNOSTIQUES " << diagnostiques << '\n';
    std::cout << "INCONNUS " << inconnus << '\n';
    return 0;
}
