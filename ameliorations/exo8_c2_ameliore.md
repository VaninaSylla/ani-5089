# Exercice 8 — L'extrapolation (Amélioré)

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

// Extrapolation SANS borne silencieuse (l'énoncé ne demande pas de borne)
Pose extrapolate(const Pose& pose, const Vec3& velLin, const Vec3& velAng, float dt) {
    Pose result;
    // Position : p + v_lin * dt
    result.pos = {
        pose.pos.x + velLin.x * dt,
        pose.pos.y + velLin.y * dt,
        pose.pos.z + velLin.z * dt
    };

    // Orientation : q * exp(0.5 * ω * dt)
    float omegaNorm = std::sqrt(velAng.x*velAng.x + velAng.y*velAng.y + velAng.z*velAng.z);
    
    if (omegaNorm < 1e-6f) {
        result.rot = pose.rot;
    } else {
        Vec3 axis = { velAng.x / omegaNorm, velAng.y / omegaNorm, velAng.z / omegaNorm };
        float angle = omegaNorm * dt;
        float half = angle * 0.5f;
        float s = std::sin(half);
        Quat qDelta = { axis.x * s, axis.y * s, axis.z * s, std::cos(half) };
        // q_new = q_old * q_delta (vitesse angulaire en repère LOCAL / body frame)
        const Quat& q = pose.rot;
        result.rot = {
            q.w * qDelta.x + q.x * qDelta.w + q.y * qDelta.z - q.z * qDelta.y,
            q.w * qDelta.y - q.x * qDelta.z + q.y * qDelta.w + q.z * qDelta.x,
            q.w * qDelta.z + q.x * qDelta.y - q.y * qDelta.x + q.z * qDelta.w,
            q.w * qDelta.w - q.x * qDelta.x - q.y * qDelta.y - q.z * qDelta.z
        };
        // Normaliser
        float n = std::sqrt(result.rot.x*result.rot.x + result.rot.y*result.rot.y +
                           result.rot.z*result.rot.z + result.rot.w*result.rot.w);
        result.rot.x /= n; result.rot.y /= n; result.rot.z /= n; result.rot.w /= n;
    }
    return result;
}

void printPose(const Pose& p, const char* label) {
    std::cout << label << ":\n";
    std::cout << std::scientific << std::setprecision(6);
    std::cout << "  pos : " << p.pos.x << ' ' << p.pos.y << ' ' << p.pos.z << '\n';
    std::cout << "  rot : " << p.rot.x << ' ' << p.rot.y << ' ' << p.rot.z << ' ' << p.rot.w << '\n';
}

int main() {
    // --- TEST 1 : Cas principal de l'énoncé ---
    // Tête à 1.70m (hauteur yeux), tourne à 180°/s, avance à 1m/s, dt=10ms
    Pose pose1 = { {0, 1.70f, 0}, {0, 0, 0, 1} }; // au sol, hauteur 1.70
    Vec3 velLin1 = { 1.0f, 0.0f, 0.0f };           // 1 m/s vers +X
    Vec3 velAng1 = { 0.0f, M_PI, 0.0f };           // 180°/s = π rad/s autour de Y
    float dt1 = 0.01f;                             // 10 ms

    // CALCUL MANUEL ATTENDU :
    // Position : (0, 1.70, 0) + (1, 0, 0) * 0.01 = (0.01, 1.70, 0) → avance de 1 cm
    // Rotation : angle = π * 0.01 = 0.0314159 rad = 1.8°
    // q_delta = [0, sin(0.9°), 0, cos(0.9°)] = [0, 0.015707, 0, 0.999877]
    // q_new = identité * q_delta = q_delta

    std::cout << "=== TEST 1 : Cas énoncé (tête 1.70m, 180°/s, 1m/s, dt=10ms) ===\n";
    std::cout << "ATTENDU pos : 1.000000e-02 1.700000e+00 0.000000e+00\n";
    std::cout << "ATTENDU rot : 0.000000e+00 1.570732e-02 0.000000e+00 9.998766e-01\n";
    
    Pose res1 = extrapolate(pose1, velLin1, velAng1, dt1);
    printPose(res1, "OBTENU");
    
    // --- TEST 2 : dt NÉGATIF (reconstruction pose passée) ---
    // Même situation, dt = -10ms → doit extrapoler DANS LE PASSÉ
    // Position : (0, 1.70, 0) + (1, 0, 0) * (-0.01) = (-0.01, 1.70, 0)
    // Rotation : angle = π * (-0.01) = -0.0314159 rad = -1.8°
    // q_delta = [0, sin(-0.9°), 0, cos(-0.9°)] = [0, -0.015707, 0, 0.999877]
    // Rotation dans l'autre sens ✓
    
    std::cout << "\n=== TEST 2 : dt NÉGATIF (-10ms, reconstruction) ===\n";
    std::cout << "ATTENDU pos : -1.000000e-02 1.700000e+00 0.000000e+00\n";
    std::cout << "ATTENDU rot : 0.000000e+00 -1.570732e-02 0.000000e+00 9.998766e-01\n";
    
    Pose res2 = extrapolate(pose1, velLin1, velAng1, -0.01f);
    printPose(res2, "OBTENU");

    // --- TEST 3 : dt = 0 ---
    std::cout << "\n=== TEST 3 : dt = 0 (identité) ===\n";
    Pose res3 = extrapolate(pose1, velLin1, velAng1, 0.0f);
    printPose(res3, "OBTENU (doit = pose initiale)");

    // --- TEST 4 : Vitesse angulaire nulle ---
    std::cout << "\n=== TEST 4 : velAng = 0 (pas de rotation) ===\n";
    Pose res4 = extrapolate(pose1, velLin1, {0,0,0}, dt1);
    printPose(res4, "OBTENU (rot = identité)");

    // --- TEST 5 : Vérifier qu'il n'y a PAS de borne silencieuse ---
    std::cout << "\n=== TEST 5 : dt = 0.5s (500ms) - PAS de clamp ===\n";
    std::cout << "ATTENDU pos : 5.000000e-01 1.700000e+00 0.000000e+00 (avance 50cm !)\n";
    std::cout << "ATTENDU rot : angle = pi*0.5 = 1.57rad = 90°\n";
    Pose res5 = extrapolate(pose1, velLin1, velAng1, 0.5f);
    printPose(res5, "OBTENU (si clampé à 0.1s, pos.x=0.10 et rot=18° -> BUG silencieux)");

    return 0;
}
```

## Résultats (EXÉCUTION RÉELLE)

### TEST 1 : Cas énoncé (tête 1.70m, 180°/s, 1m/s, dt=10ms)
```
ATTENDU pos : 1.000000e-02 1.700000e+00 0.000000e+00
ATTENDU rot : 0.000000e+00 1.570732e-02 0.000000e+00 9.998766e-01
OBTENU:
  pos : 1.000000e-02 1.700000e+00 0.000000e+00
  rot : 0.000000e+00 1.570732e-02 0.000000e+00 9.998766e-01
```
✓ **Correspondance exacte** — la fonction est démontrée.

### TEST 2 : dt NÉGATIF (-10ms, reconstruction)
```
ATTENDU pos : -1.000000e-02 1.700000e+00 0.000000e+00
ATTENDU rot : 0.000000e+00 -1.570732e-02 0.000000e+00 9.998766e-01
OBTENU:
  pos : -1.000000e-02 1.700000e+00 0.000000e+00
  rot : 0.000000e+00 -1.570732e-02 0.000000e+00 9.998766e-01
```
✓ **Rotation dans l'autre sens** (sin négatif) — gère correctement la reconstruction de pose passée.

### TEST 3 : dt = 0
```
OBTENU (doit = pose initiale):
  pos : 0.000000e+00 1.700000e+00 0.000000e+00
  rot : 0.000000e+00 0.000000e+00 0.000000e+00 1.000000e+00
```
✓ Identité.

### TEST 4 : Vitesse angulaire nulle
```
OBTENU (rot = identité):
  pos : 1.000000e-02 1.700000e+00 0.000000e+00
  rot : 0.000000e+00 0.000000e+00 0.000000e+00 1.000000e+00
```
✓ Pas de division par zéro, rotation inchangée.

### TEST 5 : dt = 0.5s (500ms) — PAS de clamp silencieux
```
ATTENDU pos : 5.000000e-01 1.700000e+00 0.000000e+00 (avance 50cm !)
ATTENDU rot : angle = pi*0.5 = 1.57rad = 90°
OBTENU:
  pos : 5.000000e-01 1.700000e+00 0.000000e+00
  rot : 0.000000e+00 7.071068e-01 0.000000e+00 7.071068e-01
```
✓ **Pas de clamp** — la fonction fait exactement ce que son nom dit : extrapolation à vitesses constantes pour n'importe quel dt. Si l'appelant veut borner, il le fait AVANT d'appeler. Sinon, surprise garantie pour celui qui appelle avec dt=0.5s en croyant avoir une extrapolation valide.

## Corrections apportées

1. **Résultats concrets affichés** — l'ancienne copie n'avait AUCUN résultat, juste des remarques. Maintenant chaque test montre ATTENDU vs OBTENU.

2. **Calcul manuel vérifié** — pour TEST 1 : position avance de 1cm, rotation 1.8° (sin(0.9°)≈0.0157, cos≈0.9999). Correspondance exacte.

3. **Test dt négatif** — cas réel (recalage mesure en retard). La fonction le gère : angle négatif → sin négatif → rotation sens inverse. ✓

4. **SUPPRESSION de la borne silencieuse à 100ms** — l'ancienne version clampait `dt` en silence. L'énoncé ne demande pas de borne. Si on limite `dt` sans le dire, la fonction ne fait plus ce qu'elle annonce. Maintenant : pas de clamp, l'appelant décide.

5. **Notation scientifique** pour voir les valeurs précises.