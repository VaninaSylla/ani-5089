#include <iostream>

namespace {

struct EtatAction {
    bool courant = false;
    bool change = false;
    bool actif = false;
};

EtatAction Actualiser(bool presse, bool lie, const EtatAction &precedent) {
    EtatAction etat;
    etat.actif = lie;
    etat.courant = etat.actif && presse;
    etat.change = etat.courant != precedent.courant;
    return etat;
}

const char *Chiffre(bool valeur) { return valeur ? "1" : "0"; }

}

int main() {
    EtatAction etat;
    bool objetAvant = true;
    bool objetApres = true;

    for (int image = 1; image <= 20; ++image) {
        const bool presse = true;
        const bool lie = image <= 5 || image >= 16;
        etat = Actualiser(presse, lie, etat);

        // Version fautive pour avant.png : elle confond une liaison perdue
        // avec un relachement, car elle ne consulte pas actif.
        if (!etat.courant && etat.change) {
            objetAvant = false;
        }

        // Version corrigee pour apres.png : seul un vrai relachement d'une
        // source encore liee peut lacher l'objet.
        if (etat.actif && !etat.courant && etat.change) {
            objetApres = false;
        }

        std::cout << "image : " << image << ", courant : " << Chiffre(etat.courant)
                  << ", change : " << Chiffre(etat.change)
                  << ", actif : " << Chiffre(etat.actif) << '\n';

        if (image == 6) {
            std::cout << "AVANT (avant.png) : objet "
                      << (objetAvant ? "TENU" : "TOMBE") << '\n';
            std::cout << "APRES (apres.png) : objet "
                      << (objetApres ? "TENU" : "TOMBE") << '\n';
        }
    }

    return 0;
}
