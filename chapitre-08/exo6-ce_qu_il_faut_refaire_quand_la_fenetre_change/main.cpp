#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n = 0;
    if (!(std::cin >> n)) return 0;

    std::string ligne;
    std::getline(std::cin, ligne);

    int survivants = 0;

    for (int i = 0; i < n; ++i) {
        std::getline(std::cin, ligne);
        if (!ligne.empty() && ligne.back() == '\r') ligne.pop_back();

        std::istringstream flux(ligne);
        std::string nom;
        flux >> nom;

        std::vector<std::string> mots;
        std::string mot;
        while (flux >> mot) mots.push_back(mot);

        int detruit = -1;
        int recree = -1;
        int reenregistre = -1;

        for (std::size_t k = 0; k < mots.size(); ++k) {
            if (mots[k] == "DETRUIT" && detruit < 0)
                detruit = static_cast<int>(k);
            else if (mots[k] == "RECREE" && recree < 0)
                recree = static_cast<int>(k);
            else if (mots[k] == "REENREGISTRE" && reenregistre < 0)
                reenregistre = static_cast<int>(k);
        }

        std::string verdict;

        if (detruit >= 0 && recree >= 0 && recree < detruit) {
            verdict = "MAUVAIS ORDRE";
        } else if (detruit < 0 || recree < 0) {
            verdict = "CHAINE PERIMEE";
        } else if (reenregistre < 0) {
            verdict = "POIGNEES PERDUES";
        } else {
            verdict = "SURVIT";
            ++survivants;
        }

        std::cout << nom << ' ' << verdict << '\n';
    }

    std::cout << "SURVIVANTS " << survivants << '\n';
    return 0;
}
