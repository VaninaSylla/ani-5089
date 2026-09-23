# Exercice 2 — La pose appliquée

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

struct Vec3 {
    float x, y, z;
};

struct Quat {
    float x, y, z, w; // (x,y,z) = partie imaginaire, w = partie réelle
};

struct Pose {
    Vec3 pos;
    Quat rot; // déjà normalisé
};

// Rotation d'un vecteur par un quaternion : v' = q * v * q^-1
// q^-1 = conjugué(q) car q est unitaire
Vec3 rotate(const Quat& q, const Vec3& v) {
    // q * v (où v = (v.x, v.y, v.z, 0))
    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;
    float vx = v.x, vy = v.y, vz = v.z;

    // t = 2 * (q.xyz × v)
    float tx = 2.0f * (qy * vz - qz * vy);
    float ty = 2.0f * (qz * vx - qx * vz);
    float tz = 2.0f * (qx * vy - qy * vx);

    // v' = v + qw * t + q.xyz × t
    return {
        vx + qw * tx + (qy * tz - qz * ty),
        vy + qw * ty + (qz * tx - qx * tz),
        vz + qw * tz + (qx * ty - qy * tx)
    };
}

// Application de la pose : rotation PUIS translation
// p_espace = orientation * p_entite + position
Vec3 applyPose(const Pose& pose, const Vec3& p_local) {
    Vec3 rotated = rotate(pose.rot, p_local);
    return {
        rotated.x + pose.pos.x,
        rotated.y + pose.pos.y,
        rotated.z + pose.pos.z
    };
}

int main() {
    Pose pose;
    Vec3 p;

    // Lecture : pos.x pos.y pos.z  rot.x rot.y rot.z rot.w  p.x p.y p.z
    std::cin >> pose.pos.x >> pose.pos.y >> pose.pos.z
             >> pose.rot.x >> pose.rot.y >> pose.rot.z >> pose.rot.w
             >> p.x >> p.y >> p.z;

    Vec3 result = applyPose(pose, p);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << result.x << ' ' << result.y << ' ' << result.z << '\n';
    return 0;
}
```

## Explications

- Structure `Pose` = position (Vec3) + orientation (Quat unitaire). **Pas d'échelle** — le chapitre dit : « une pose n'en a pas, la raison est physique : un casque ne redimensionne pas la tête de son porteur. »
- Ordre : **rotation puis translation** (formule du chapitre : `p_espace = orientation * p_entite + position`).
- Rotation par quaternion : formule optimisée `v' = v + w*t + q×t` où `t = 2*(q×v)`. Pas de matrice 4x4, plus rapide et plus stable.
- Le quaternion est déjà normalisé (donné par l'énoncé).

## Test

Pose : position (1, 2, 3), rotation identité (0, 0, 0, 1)
Point : (4, 5, 6)
Résultat attendu : (5, 7, 9) = (1+4, 2+5, 3+6)

Input : `1 2 3  0 0 0 1  4 5 6`
Output : `5.0000 7.0000 9.0000` ✓

Rotation de 90° autour de Y (quaternion ~ 0, 0.707, 0, 0.707), point (1, 0, 0) → devrait donner (0, 0, -1) + position.
Ça marche.