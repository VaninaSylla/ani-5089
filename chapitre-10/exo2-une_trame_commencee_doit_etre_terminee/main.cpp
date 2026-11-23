#include <iostream>
#include <string>

int main() {
    int n;
    std::cin >> n;
    std::string etat = "REPOS";
    int fautes = 0;
    int bloque = 0;

    for (int i = 1; i <= n; i++) {
        std::string geste;
        std::cin >> geste;
        const char *verdict = "OK";

        if (geste == "ATTEND") {
            if (etat == "REPOS") {
                etat = "ATTENDUE";
            } else if (etat == "ATTENDUE") {
                verdict = "DEJA ATTENDUE";
                fautes++;
            } else {
                verdict = "TRAME EN COURS";
                fautes++;
                if (bloque == 0) {
                    bloque = i;
                }
            }
        } else if (geste == "COMMENCE") {
            if (etat == "ATTENDUE") {
                etat = "COMMENCEE";
            } else if (etat == "REPOS") {
                verdict = "PAS ATTENDUE";
                fautes++;
            } else {
                verdict = "DEJA COMMENCEE";
                fautes++;
            }
        } else if (geste == "FINIT") {
            if (etat == "COMMENCEE") {
                etat = "REPOS";
            } else {
                verdict = "PAS COMMENCEE";
                fautes++;
            }
        }

        std::cout << i << " " << geste << " " << verdict << '\n';
    }

    std::cout << "FAUTES " << fautes << '\n';
    std::cout << "BLOQUE " << bloque << '\n';
    return 0;
}
