# Exercice 8 — L'extrapolation

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

// ... Vec3, Quat, Pose, rotate, applyPose, inversePose, compose, quatFromAxisAngle ...

// Extrapolation d'une pose à vitesses constantes (linéaire + angulaire)
// pose_extrapolée = pose avancée de dt secondes
Pose extrapolate(const Pose& pose, const Vec3& velLin, const Vec3& velAng, float dt) {
    // Bornes du module : dt ≤ 100 ms = 0.1 s
    if (dt > 0.1f) dt = 0.1f;
    if (dt < 0.0f) dt = 0.0f;

    Pose result;
    // Position : p + v_lin * dt
    result.pos = {
        pose.pos.x + velLin.x * dt,
        pose.pos.y + velLin.y * dt,
        pose.pos.z + velLin.z * dt
    };

    // Orientation : q * exp(0.5 * ω * dt)
    // ω = vitesse angulaire (vecteur, norme = vitesse en rad/s)
    // Si ω = 0, pas de rotation
    float omegaNorm = std::sqrt(velAng.x*velAng.x + velAng.y*velAng.y + velAng.z*velAng.z);
    
    if (omegaNorm < 1e-6f) {
        // Vitesse angulaire nulle → pas de changement de rotation
        result.rot = pose.rot;
    } else {
        // Axe normalisé
        Vec3 axis = { velAng.x / omegaNorm, velAng.y / omegaNorm, velAng.z / omegaNorm };
        // Angle = omegaNorm * dt
        float angle = omegaNorm * dt;
        // Quaternion de rotation : q_delta = [axis * sin(angle/2), cos(angle/2)]
        float half = angle * 0.5f;
        float s = std::sin(half);
        Quat qDelta = { axis.x * s, axis.y * s, axis.z * s, std::cos(half) };
        // Composer : q_new = q_delta * q_old (rotation delta DANS le repère local)
        // Wait: l'extrapolation c'est rotation dans le repère monde ou local ?
        // Le chapitre dit "vitesse angulaire en radians par seconde" - c'est dans quel repère ?
        // Typiquement vitesse angulaire est dans le repère local (body frame).
        // Donc q_new = q_old * q_delta (appliquer delta après la rotation actuelle)
        const Quat& q = pose.rot;
        result.rot = {
            q.w * qDelta.x + q.x * qDelta.w + q.y * qDelta.z - q.z * qDelta.y,
            q.w * qDelta.y - q.x * qDelta.z + q.y * qDelta.w + q.z * qDelta.x,
            q.w * qDelta.z + q.x * qDelta.y - q.y * qDelta.x + q.z * qDelta.w,
            q.w * qDelta.w - q.x * qDelta.x - q.y * qDelta.y - q.z * qDelta.z
        };
        // Normaliser par sécurité
        float n = std::sqrt(result.rot.x*result.rot.x + result.rot.y*result.rot.y +
                           result.rot.z*result.rot.z + result.rot.w*result.rot.w);
        result.rot.x /= n; result.rot.y /= n; result.rot.z /= n; result.rot.w /= n;
    }

    return result;
}

int main() {
    Pose pose;
    Vec3 velLin, velAng;
    float dt;

    // Lecture : pos.x pos.y pos.z  rot.x rot.y rot.z rot.w  velLin.x velLin.y velLin.z  velAng.x velAng.y velAng.z  dt
    std::cin >> pose.pos.x >> pose.pos.y >> pose.pos.z
             >> pose.rot.x >> pose.rot.y >> pose.rot.z >> pose.rot.w
             >> velLin.x >> velLin.y >> velLin.z
             >> velAng.x >> velAng.y >> velAng.z
             >> dt;

    Pose extrap = extrapolate(pose, velLin, velAng, dt);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Position : " << extrap.pos.x << ' ' << extrap.pos.y << ' ' << extrap.pos.z << '\n';
    std::cout << "Rotation : " << extrap.rot.x << ' ' << extrap.rot.y << ' ' << extrap.rot.z << ' ' << extrap.rot.w << '\n';

    return 0;
}
```

## Tests

### Test 1 : Translation seule
Pose : (0,0,0), rot identité
velLin : (1, 2, 3) m/s
velAng : (0, 0, 0)
dt : 0.05 s (50 ms)

```
Position : 0.050000 0.100000 0.150000
Rotation : 0.000000 0.000000 0.000000 1.000000
```
→ (0+1*0.05, 0+2*0.05, 0+3*0.05) = (0.05, 0.1, 0.15) ✓
Rotation inchangée ✓

### Test 2 : Rotation seule (90°/s autour de Y)
Pose : (0,0,0), rot identité
velLin : (0,0,0)
velAng : (0, π/2, 0) ≈ (0, 1.5708, 0) rad/s = 90°/s
dt : 0.1 s (100 ms, borne max)

Angle = 1.5708 * 0.1 = 0.15708 rad = 9°
```
Rotation : 0.000000 0.078459 0.000000 0.996917
```
sin(4.5°) ≈ 0.0785, cos(4.5°) ≈ 0.9969 ✓

### Test 3 : Vitesse angulaire nulle (division par zéro évitée)
velAng : (0, 0, 0)
→ omegaNorm < 1e-6 → branche `if` prise → rotation inchangée. ✓ Pas de division par zéro.

### Test 4 : Borne 100 ms
dt : 0.5 s (500 ms) → clampé à 0.1 s
Le chapitre : « au-delà de cent millisecondes, le modèle à vitesses constantes ment plus qu'il n'aide ; les vrais runtimes bornent pareil. »

## Remarques

- Intégration d'Euler simple : vitesse d'abord, position ensuite (comme dit le chapitre).
- Borne à 100 ms respectée.
- Cas vitesse angulaire nulle géré sans division par zéro.
- Quaternion normalisé après composition (sécurité).

Le module du parcours fait pareil : « il ne prétend pas prédire aussi bien qu'un vrai casque, il annonce ce qu'il fait, dit ce qu'il ne fait pas, et garantit que la forme de l'appel est la bonne. »