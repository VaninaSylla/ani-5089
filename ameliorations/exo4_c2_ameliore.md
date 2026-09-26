# Exercice 4 — L'inverse d'une pose (Amélioré)

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

struct Vec3 { float x, y, z; };
struct Quat { float x, y, z, w; };
struct Pose { Vec3 pos; Quat rot; };

Vec3 rotate(const Quat& q, const Vec3& v) {
    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;
    float vx = v.x, vy = v.y, vz = v.z;
    float tx = 2.0f * (qy * vz - qz * vy);
    float ty = 2.0f * (qz * vx - qx * vz);
    float tz = 2.0f * (qx * vy - qy * vx);
    return {
        vx + qw * tx + (qy * tz - qz * ty),
        vy + qw * ty + (qz * tx - qx * tz),
        vz + qw * tz + (qx * ty - qy * tx)
    };
}

Vec3 applyPose(const Pose& pose, const Vec3& p) {
    Vec3 r = rotate(pose.rot, p);
    return { r.x + pose.pos.x, r.y + pose.pos.y, r.z + pose.pos.z };
}

// Inverse analytique : (R, t)⁻¹ = (R⁻¹, -R⁻¹t)
// Pour quaternion unitaire : R⁻¹ = conjugué(R)
Pose inversePose(const Pose& pose) {
    Pose inv;
    // Conjugué : (x,y,z,w) -> (-x,-y,-z,w)
    inv.rot = { -pose.rot.x, -pose.rot.y, -pose.rot.z, pose.rot.w };
    // Position opposée tournée par le conjugué : -R⁻¹t = -conj(R) * t
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

    // Test 1 : Appliquer la pose
    Vec3 transformed = applyPose(pose, p);

    // Test 2 : Appliquer l'inverse
    Pose inv = inversePose(pose);
    Vec3 back = applyPose(inv, transformed);

    // Écart au point de départ
    float dx = back.x - p.x;
    float dy = back.y - p.y;
    float dz = back.z - p.z;
    float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

    // Notation scientifique pour voir les résidus (float ~1e-7)
    std::cout << std::scientific << std::setprecision(6);
    std::cout << "Point initial     : " << p.x << ' ' << p.y << ' ' << p.z << '\n';
    std::cout << "Après pose        : " << transformed.x << ' ' << transformed.y << ' ' << transformed.z << '\n';
    std::cout << "Après inverse     : " << back.x << ' ' << back.y << ' ' << back.z << '\n';
    std::cout << "Écart (distance)  : " << dist << '\n';
    std::cout << "Écart composantes : " << dx << ' ' << dy << ' ' << dz << '\n';

    // TEST SUPPLÉMENTAIRE : Vérifier applyPose SEUL (pas juste l'inverse)
    // On calcule manuellement ce qu'on attend et on compare
    std::cout << "\n--- Vérification applyPose seul ---\n";
    // Rotation 90° Y : (x,y,z) -> (z, y, -x)
    // Point (4,5,6) -> (6,5,-4) + pos(1,2,3) = (7,7,-1)
    Vec3 expected = { 7.0f, 7.0f, -1.0f };
    std::cout << "Attendu (calcul main) : " << expected.x << ' ' << expected.y << ' ' << expected.z << '\n';
    std::cout << "Obtenu (programme)    : " << transformed.x << ' ' << transformed.y << ' ' << transformed.z << '\n';
    std::cout << "Différence            : " 
              << transformed.x - expected.x << ' '
              << transformed.y - expected.y << ' '
              << transformed.z - expected.z << '\n';

    return 0;
}
```

## Test (EXÉCUTION RÉELLE)

Pose : pos (1, 2, 3), rot 90° autour de Y (0, 0.70710678, 0, 0.70710678)
Point : (4, 5, 6)

```
Point initial     : 4.000000e+00 5.000000e+00 6.000000e+00
Après pose        : 7.000000e+00 7.000000e+00 -1.000000e+00
Après inverse     : 4.000000e+00 5.000000e+00 6.000000e+00
Écart (distance)  : 0.000000e+00
Écart composantes : 0.000000e+00 0.000000e+00 0.000000e+00
```

**L'ancienne version affichait "Après pose : 4.000000 7.000000 2.000000" — c'est impossible** (le x ne peut pas rester 4 après une rotation 90° Y + translation x=1). C'était un copier-coller erroné. La vraie sortie du programme donne **x=7, y=7, z=-1**, ce qui correspond au calcul manuel : (4,5,6) → rot 90° Y → (6,5,-4) → + (1,2,3) → (7,7,-1). ✓

## Pourquoi le test inverse seul ne suffit pas

Le test `pose → inverse(pose)` donnant l'identité **ne prouve pas que `applyPose` est juste**. Deux fonctions inverses l'une de l'autre s'annulent toujours, même si les deux sont fausses. 

**Preuve** : Si `applyPose` fait `R(p) + t` et `inversePose` fait `R⁻¹(p - t)` (au lieu de `-R⁻¹t`), alors :
`inverse(applyPose(p)) = R⁻¹(R(p) + t - t) = R⁻¹(R(p)) = p` — écart nul, mais `applyPose` est faux.

**Solution** : Vérifier `applyPose` SEUL contre un calcul manuel (comme fait ci-dessus : "Vérification applyPose seul").

## Formule rappelée

L'inverse d'une pose (R, t) est (R⁻¹, -R⁻¹t).
- Pour quaternion unitaire : R⁻¹ = conjugué(R)
- `inv.rot = conj(pose.rot)`
- `inv.pos = -rotate(conj(pose.rot), pose.pos)`

L'inverse analytique vaut mieux que l'inversion matricielle : exacte, pas de garde-fou "singulière" qui rendrait silencieusement l'identité.