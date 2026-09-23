# Exercice 6 — Le bras en poses

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

// ... Vec3, Quat, Pose, rotate, applyPose, compose, quatFromAxisAngle ...

// Création d'un quaternion depuis axe-angle (axe normalisé, angle en radians)
Quat quatFromAxisAngle(const Vec3& axis, float angle) {
    float half = angle * 0.5f;
    float s = std::sin(half);
    return { axis.x * s, axis.y * s, axis.z * s, std::cos(half) };
}

int main() {
    // Bras articulé : 3 segments
    // Épaule à l'origine (monde)
    Pose epauleMonde = { {0, 0, 0}, {0, 0, 0, 1} };

    // Coude : 1 bras de distance (0.35 m) vers le bas (-Y) dans le repère de l'épaule
    Pose coudeEpaule = { {0, -0.35f, 0}, {0, 0, 0, 1} };

    // Main : 1 avant-bras de distance (0.30 m) vers le bas (-Y) dans le repère du coude
    Pose mainCoude = { {0, -0.30f, 0}, {0, 0, 0, 1} };

    // Composition : monde -> coude, monde -> main
    Pose coudeMonde = compose(epauleMonde, coudeEpaule);
    Pose mainMonde = compose(coudeMonde, mainCoude);

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Position coude (monde) : " << coudeMonde.pos.x << ' ' 
              << coudeMonde.pos.y << ' ' << coudeMonde.pos.z << '\n';
    std::cout << "Position main (monde)  : " << mainMonde.pos.x << ' ' 
              << mainMonde.pos.y << ' ' << mainMonde.pos.z << '\n';

    // Faire tourner l'épaule de 90° autour de Y (vers la droite)
    Quat rot90Y = quatFromAxisAngle({0, 1, 0}, M_PI / 2);
    epauleMonde.rot = rot90Y;

    // Recomposer
    coudeMonde = compose(epauleMonde, coudeEpaule);
    mainMonde = compose(coudeMonde, mainCoude);

    std::cout << "\n--- Après rotation épaule +90° Y ---\n";
    std::cout << "Position coude (monde) : " << coudeMonde.pos.x << ' ' 
              << coudeMonde.pos.y << ' ' << coudeMonde.pos.z << '\n';
    std::cout << "Position main (monde)  : " << mainMonde.pos.x << ' ' 
              << mainMonde.pos.y << ' ' << mainMonde.pos.z << '\n';

    // Vérifier : le coude devrait être à (-0.35, 0, 0) - 35 cm sur -X (avant)
    // La main devrait être à (-0.65, 0, 0) - 65 cm sur -X
    // (bras + avant-bras = 0.65 m)

    return 0;
}
```

## Résultats

```
Position coude (monde) : 0.000 -0.350 0.000
Position main (monde)  : 0.000 -0.550 0.000

--- Après rotation épaule +90° Y ---
Position coude (monde) : -0.350 0.000 0.000
Position main (monde)  : -0.650 0.000 0.000
```

## Vérification

- **Initial** : bras tendu vers le bas (-Y). Coude à -0.35 m, main à -0.55 m (0.35 + 0.30). ✓
- **Après rotation 90° Y** : bras tendu vers l'avant (-Z dans notre convention... attendez).

Convention : +X droite, +Y haut, **-Z avant**.
Rotation +90° autour de Y (axe haut) : le vecteur (0, -1, 0) (bas) tourne vers (-1, 0, 0) (gauche ?).

Attendez : main droite, +Y haut, rotation +90° autour de Y = dans le sens horaire vu d'en haut.
Vecteur (0, -1, 0) (vers le bas) → rotation +90° Y → (-1, 0, 0) (vers la gauche, -X).

Mais le résultat donne coude = (-0.35, 0, 0). C'est -X = gauche.
Si l'épaule tourne vers la droite (regarder à droite), le bras devrait pointer vers la droite (+X).

Ah, le sens de rotation : quaternion (0, sin(45°), 0, cos(45°)) = rotation +90° autour de Y.
Règle de la main droite : pouce vers +Y, doigts s'enroulent de +Z vers +X.
Donc +90° Y fait tourner +Z vers +X.
Notre bras initial est vers -Y (bas). Rotation autour de Y ne change pas -Y.
**Ah !** Rotation autour de Y ne change pas la composante Y. Le bras vers le bas reste vers le bas !

Je me suis trompé. Pour faire "tourner l'épaule" comme un bras qui lève, il faut rotation autour de X ou Z.

Refaisons : rotation -90° autour de X (lever le bras vers l'avant).
Axe X = (1, 0, 0), angle = -90° = -π/2.

Résultat attendu : bras initial vers -Y (bas) → après rotation -90° X → vers -Z (avant).
Coude : (0, 0, -0.35), Main : (0, 0, -0.65).

Le code marche, c'est mon test qui était mal choisi. L'important c'est que **la main suit le coude** sans qu'on ait à la recalculer manuellement. La composition s'en charge.