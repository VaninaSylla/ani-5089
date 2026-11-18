#include "actions.h"

#include <iostream>

namespace {

struct Entree {
    bool presse;
    bool lie;
};

const char *Chiffre(bool valeur) { return valeur ? "1" : "0"; }

}

int main() {
    const Entree saisir[6] = {{false, true}, {true, true},  {true, true},
                              {true, true},  {false, true}, {false, true}};
    const Entree tirer[6] = {{false, true}, {false, true}, {true, true},
                             {false, true}, {true, true},  {false, true}};
    const Entree avancer[6] = {{true, false},  {false, false}, {true, false},
                               {false, false}, {true, false},  {false, false}};

    EtatAction etatSaisir;
    EtatAction etatTirer;
    EtatAction etatAvancer;

    std::cout << "image | saisir.courant saisir.change saisir.actif | "
                 "tirer.courant tirer.change tirer.actif | "
                 "avancer.courant avancer.change avancer.actif\n";

    for (int image = 0; image < 6; ++image) {
        etatSaisir = Actualiser(saisir[image].presse, saisir[image].lie, etatSaisir);
        etatTirer = Actualiser(tirer[image].presse, tirer[image].lie, etatTirer);
        etatAvancer = Actualiser(avancer[image].presse, avancer[image].lie, etatAvancer);

        std::cout << image + 1 << " | "
                  << Chiffre(etatSaisir.courant) << ' '
                  << Chiffre(etatSaisir.change) << ' '
                  << Chiffre(etatSaisir.actif) << " | "
                  << Chiffre(etatTirer.courant) << ' '
                  << Chiffre(etatTirer.change) << ' '
                  << Chiffre(etatTirer.actif) << " | "
                  << Chiffre(etatAvancer.courant) << ' '
                  << Chiffre(etatAvancer.change) << ' '
                  << Chiffre(etatAvancer.actif) << '\n';
    }

    return 0;
}
