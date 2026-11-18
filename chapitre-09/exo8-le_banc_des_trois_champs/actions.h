#ifndef ACTIONS_H
#define ACTIONS_H

struct EtatAction {
    bool courant = false;
    bool change = false;
    bool actif = false;
};

inline EtatAction Actualiser(bool presse, bool lie, const EtatAction &precedent) {
    EtatAction etat;
    etat.actif = lie;
    etat.courant = etat.actif && presse;
    etat.change = etat.courant != precedent.courant;
    return etat;
}

#endif
