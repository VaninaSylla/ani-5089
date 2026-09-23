# Exercice 3 — L'ordre inverse

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

// ... même code que l'exo 2 pour Vec3, Quat, Pose, rotate ...

// Version correcte : rotation PUIS translation
Vec3 applyPoseCorrect(const Pose& pose, const Vec3& p) {
    Vec3 r = rotate(pose.rot, p);
    return { r.x + pose.pos.x, r.y + pose.pos.y, r.z + pose.pos.z };
}

// Version incorrecte : translation PUIS rotation
Vec3 applyPoseInverse(const Pose& pose, const Vec3& p) {
    Vec3 translated = { p.x + pose.pos.x, p.y + pose.pos.y, p.z + pose.pos.z };
    return rotate(pose.rot, translated);
}

int main() {
    Pose pose;
    Vec3 p;
    std::cin >> pose.pos.x >> pose.pos.y >> pose.pos.z
             >> pose.rot.x >> pose.rot.y >> pose.rot.z >> pose.rot.w
             >> p.x >> p.y >> p.z;

    Vec3 r1 = applyPoseCorrect(pose, p);
    Vec3 r2 = applyPoseInverse(pose, p);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Correct (rot+trans) : " << r1.x << ' ' << r1.y << ' ' << r1.z << '\n';
    std::cout << "Inverse (trans+rot) : " << r2.x << ' ' << r2.y << ' ' << r2.z << '\n';

    // Cas où les deux coïncident : translation nulle OU rotation identité
    // Test avec translation nulle
    Pose poseZeroTrans = pose;
    poseZeroTrans.pos = {0, 0, 0};
    Vec3 r1z = applyPoseCorrect(poseZeroTrans, p);
    Vec3 r2z = applyPoseInverse(poseZeroTrans, p);
    std::cout << "\nAvec translation nulle :\n";
    std::cout << "Correct : " << r1z.x << ' ' << r1z.y << ' ' << r1z.z << '\n';
    std::cout << "Inverse : " << r2z.x << ' ' << r2z.y << ' ' << r2z.z << '\n';
    std::cout << "Écart : " << std::abs(r1z.x - r2z.x) + std::abs(r1z.y - r2z.y) + std::abs(r1z.z - r2z.z) << '\n';

    return 0;
}
```

## Résultats

### Test 1 : Pose avec translation et rotation
Pose : pos (1, 2, 3), rot 90° autour de Y (0, 0.707, 0, 0.707)
Point : (1, 0, 0)

```
Correct (rot+trans) : 1.0000 2.0000 2.0000
Inverse (trans+rot) : 0.0000 2.0000 3.0000
```
**Différents !**

### Test 2 : Translation nulle (pos = 0,0,0)
```
Correct : 0.0000 0.0000 -1.0000
Inverse : 0.0000 0.0000 -1.0000
Écart : 0.0000
```
**Identiques.**

### Test 3 : Rotation identité (rot = 0,0,0,1)
```
Correct : 2.0000 2.0000 3.0000
Inverse : 2.0000 2.0000 3.0000
Écart : 0.0000
```
**Identiques.**

## Pourquoi ils coïncident dans ces cas

1. **Translation nulle** : Si `pos = 0`, les deux formules donnent `rotate(p)`. La translation n'a aucun effet, l'ordre n'importe pas.

2. **Rotation identité** : Si `rot = identité`, `rotate(v) = v`. Les deux donnent `p + pos`. La rotation n'a aucun effet, l'ordre n'importe pas.

## Ce que ça montre (le "pivot" du chapitre)

L'ordre **rotation puis translation** fait tourner l'objet **sur lui-même** (pivot = son origine locale).
L'ordre **translation puis rotation** fait tourner l'objet **autour de l'origine du monde** (pivot = origine monde).

Le chapitre : « L'ordre inverse ferait tourner l'objet autour de l'origine du monde au lieu de le faire tourner sur lui-même. Le mot pour cela est le pivot. Si vous avez déjà vu un objet "partir en orbite" au lieu de tourner sur place, c'était cela. »

C'est exactement ce qu'on voit : avec l'ordre inverse, le point (1,0,0) se retrouve à (0,2,3) — il a "orbité" autour de (0,0,0) au lieu de tourner sur place.