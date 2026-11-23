#ifndef COUCHES_H
#define COUCHES_H

#include <string>

struct Vue {
    bool presente;
};

struct Couche {
    std::string espace;
    Vue gauche;
    Vue droite;
};

#endif
