#include <iostream>
#include <string>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n = 0;
    if (!(std::cin >> n)) return 0;

    bool acquise = false;
    bool rendue = false;
    int fautes = 0;

    for (int tour = 1; tour <= n; ++tour) {
        std::string appel;
        std::cin >> appel;

        std::string verdict = "OK";

        if (appel == "ACQUIERT") {
            if (acquise) {
                verdict = "DEJA ACQUISE";
                ++fautes;
            } else {
                acquise = true;
            }
        } else if (appel == "DESSINE") {
            if (!acquise) {
                verdict = "PAS ACQUISE";
                ++fautes;
            }
        } else if (appel == "REND") {
            if (!acquise) {
                verdict = "RIEN ACQUIS";
                ++fautes;
            } else {
                acquise = false;
                rendue = true;
            }
        } else if (appel == "SOUMET") {
            if (!rendue) {
                verdict = "PAS RENDUE";
                ++fautes;
            } else {
                rendue = false;
            }
        }

        std::cout << tour << ' ' << appel << ' ' << verdict << '\n';
    }

    std::cout << "FAUTES " << fautes << '\n';
    return 0;
}
