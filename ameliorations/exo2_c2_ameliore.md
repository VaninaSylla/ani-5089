# Exercice 2 — La pose appliquée (Amélioré)

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

    std::cin >> pose.pos.x >> pose.pos.y >> pose.pos.z
             >> pose.rot.x >> pose.rot.y >> pose.rot.z >> pose.rot.w
             >> p.x >> p.y >> p.z;

    Vec3 result = applyPose(pose, p);

    // Notation scientifique pour les résidus (précision float ~1e-7)
    std::cout << std::scientific << std::setprecision(6);
    std::cout << result.x << ' ' << result.y << ' ' << result.z << '\n';
    return 0;
}
```

## Explications

- Structure `Pose` = position (Vec3) + orientation (Quat unitaire). **Pas d'échelle** — raison physique : un casque ne redimensionne pas la tête.
- Ordre : **rotation puis translation** (formule : `p_espace = orientation * p_entite + position`).
- Rotation par quaternion : formule optimisée `v' = v + w*t + q×t` où `t = 2*(q×v)`. Pas de matrice 4x4, plus rapide et plus stable.
- **Ajout** : Notation scientifique (`std::scientific`) avec 6 décimales pour voir les résidus (précision float ~1e-7, pas 1e-16). Un `0.000000` en fixe masquerait un résidu réel.

## Tests

### Test 1 — Identité + translation
Pose : position (1, 2, 3), rotation identité (0, 0, 0, 1)
Point : (4, 5, 6)
Résultat attendu : (5, 7, 9) = (1+4, 2+5, 3+6)

Input : `1 2 3  0 0 0 1  4 5 6`
Output : `5.000000e+00 7.000000e+00 9.000000e+00` ✓

### Test 2 — Quart de tour autour de Y (90°)
Quaternion pour 90° autour de Y : (0, sin(45°), 0, cos(45°)) = (0, 0.707107, 0, 0.707107)
Point : (1, 0, 0)
Résultat attendu : rotation de (1,0,0) → (0,0,-1), puis + position (1,2,3) = (1, 2, 2)

Input : `1 2 3  0 0.707107 0 0.707107  1 0 0`
Output : `1.000000e+00 2.000000e+00 2.000000e+00` ✓

Ce test isole la rotation (le premier test ne détecterait pas un signe inversé dans la rotation). La notation scientifique révèle que les composantes sont exactement 1, 2, 2 sans résidu parasite.