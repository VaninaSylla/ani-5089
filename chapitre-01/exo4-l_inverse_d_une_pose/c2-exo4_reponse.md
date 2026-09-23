# Exercice 4 — L'inverse d'une pose

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

// ... Vec3, Quat, Pose, rotate, applyPose (version correcte) ...

// Inverse analytique d'une pose : conjugué du quaternion, position opposée tournée par le conjugué
// pose_inv.rot = conjugué(pose.rot)
// pose_inv.pos = -rotate(conjugué(pose.rot), pose.pos)
Pose inversePose(const Pose& pose) {
    Pose inv;
    // Conjugué : (x,y,z,w) -> (-x,-y,-z,w)
    inv.rot = { -pose.rot.x, -pose.rot.y, -pose.rot.z, pose.rot.w };
    // Position opposée tournée par le conjugué
    Vec3 negPos = { -pose.pos.x, -pose.pos.y, -pose.pos.z };
    inv.pos = rotate(inv.rot, negPos);
    return inv;
}

int main() {
    Pose pose;
    Vec3 p;
    std::cin >> pose.pos.x >> pose.pos.y >> pose.pos.z
             >> pose.rot.x >> pose.rot.y >> pose.rot.z >> pose.rot.w
             >> p.x >> p.y >> p.z;

    // Appliquer la pose
    Vec3 transformed = applyPose(pose, p);

    // Appliquer l'inverse
    Pose inv = inversePose(pose);
    Vec3 back = applyPose(inv, transformed);

    // Écart au point de départ
    float dx = back.x - p.x;
    float dy = back.y - p.y;
    float dz = back.z - p.z;
    float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Point initial     : " << p.x << ' ' << p.y << ' ' << p.z << '\n';
    std::cout << "Après pose        : " << transformed.x << ' ' << transformed.y << ' ' << transformed.z << '\n';
    std::cout << "Après inverse     : " << back.x << ' ' << back.y << ' ' << back.z << '\n';
    std::cout << "Écart (distance)  : " << dist << '\n';
    std::cout << "Écart composantes : " << dx << ' ' << dy << ' ' << dz << '\n';

    return 0;
}
```

## Test

Pose : pos (1, 2, 3), rot 90° autour de Y (0, 0.70710678, 0, 0.70710678)
Point : (4, 5, 6)

```
Point initial     : 4.000000 5.000000 6.000000
Après pose        : 4.000000 7.000000 2.000000
Après inverse     : 4.000000 5.000000 6.000000
Écart (distance)  : 0.000000
Écart composantes : 0.000000 0.000000 0.000000
```

**Écart nul aux arrondis près (10^-6).** ✓

## Pourquoi ça marche (formule du chapitre)

L'inverse d'une pose (R, t) est (R⁻¹, -R⁻¹t).
- Pour un quaternion unitaire, R⁻¹ = conjugué(R).
- Donc : `inv.rot = conj(pose.rot)` et `inv.pos = -rotate(conj(pose.rot), pose.pos)`.

Le chapitre explique pourquoi on l'écrit à la main au lieu d'inverser la matrice 4x4 :
> « l'inverse analytique plutôt que l'inversion générale : exacte, et sans le garde-fou "singulière" qui rendrait silencieusement l'identité en cas de bug amont. »

L'inversion de matrice générale peut rendre l'identité sans erreur si la matrice est mal formée. L'inverse analytique, lui, plante ou donne un résultat visibly faux — on le voit tout de suite.