#include <iostream>
#include <cstdlib>

int main() {
    int n;
    std::cin >> n;
    int justes = 0;
    int pires = 0;

    for (int i = 0; i < n; i++) {
        int rendu, reel, soumis;
        std::cin >> rendu >> reel >> soumis;
        int sans = rendu - reel;
        int avec = rendu - soumis;
        const char *verdict;

        if (avec == 0) {
            verdict = "JUSTE";
            justes++;
        } else if (std::abs(avec) < std::abs(sans)) {
            verdict = "PARTIEL";
        } else if (std::abs(avec) == std::abs(sans)) {
            verdict = "SANS EFFET";
        } else {
            verdict = "PIRE";
            pires++;
        }

        std::cout << "SANS " << sans << " AVEC " << avec << " " << verdict << '\n';
    }

    std::cout << "JUSTES " << justes << '\n';
    std::cout << "PIRES " << pires << '\n';
    return 0;
}
