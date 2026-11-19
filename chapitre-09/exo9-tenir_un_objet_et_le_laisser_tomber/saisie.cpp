#include <iomanip>
#include <iostream>

namespace {

struct Pose {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Pose Inverted() const { return Pose{-x, -y, -z}; }
};

Pose operator*(const Pose &a, const Pose &b) {
    return Pose{a.x + b.x, a.y + b.y, a.z + b.z};
}

struct EtatAction {
    bool current = false;
    bool changed = false;
    bool active = true;
};

struct Resultat {
    Pose pose;
    bool tenu = false;
};

Resultat Simuler(bool tenirSurFront) {
    EtatAction saisir;
    bool precedent = false;
    Pose cube{0.40f, 0.80f, -0.50f};
    bool tenu = false;
    Pose attache;

    for (int image = 1; image <= 3; ++image) {
        const bool presse = image >= 2;
        saisir.current = saisir.active && presse;
        saisir.changed = saisir.current != precedent;
        precedent = saisir.current;

        const Pose posePaume{0.20f + 0.10f * image, 1.10f, -0.30f};

        if (saisir.current && saisir.changed) {
            attache = posePaume.Inverted() * cube;
            tenu = true;
        }

        // Version fautive pour front.png : current && changed ne vaut vrai
        // qu'au front. A l'image suivante le cube n'est plus tenu.
        if (tenirSurFront) {
            if (!(saisir.current && saisir.changed)) {
                tenu = false;
                cube.y -= 0.18f;
            }
        } else if (saisir.current) {
            // Version juste pour niveau.png : le niveau maintient le cube.
            cube = posePaume * attache;
            tenu = true;
        }
    }

    return Resultat{cube, tenu};
}

void Afficher(const char *titre, const Resultat &resultat) {
    std::cout << titre << " - image 3, doigt encore appuye\n"
              << "cube : " << (resultat.tenu ? "TENU" : "TOMBE") << '\n'
              << "position cube : (" << resultat.pose.x << ", " << resultat.pose.y
              << ", " << resultat.pose.z << ")\n";
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(2);
    Afficher("FRONT (front.png)", Simuler(true));
    std::cout << '\n';
    Afficher("NIVEAU (niveau.png)", Simuler(false));
    return 0;
}
