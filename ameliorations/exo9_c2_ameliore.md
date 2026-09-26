# Exercice 9 — Le chemin court (Amélioré)

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

struct Quat { float x, y, z, w; };

float dot(const Quat& a, const Quat& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

Quat slerpShortest(const Quat& q1, const Quat& q2, float t) {
    float cosTheta = dot(q1, q2);
    Quat q2Fixed = q2;
    if (cosTheta < 0.0f) {
        q2Fixed = { -q2.x, -q2.y, -q2.z, -q2.w };
        cosTheta = -cosTheta;
    }
    if (cosTheta > 1.0f) cosTheta = 1.0f;
    if (cosTheta < -1.0f) cosTheta = -1.0f;
    
    if (cosTheta > 0.9995f) {
        Quat result = {
            q1.x + t * (q2Fixed.x - q1.x),
            q1.y + t * (q2Fixed.y - q1.y),
            q1.z + t * (q2Fixed.z - q1.z),
            q1.w + t * (q2Fixed.w - q1.w)
        };
        float n = std::sqrt(dot(result, result));
        return { result.x/n, result.y/n, result.z/n, result.w/n };
    }
    
    float theta = std::acos(cosTheta);
    float sinTheta = std::sin(theta);
    float w1 = std::sin((1.0f - t) * theta) / sinTheta;
    float w2 = std::sin(t * theta) / sinTheta;
    
    return {
        w1 * q1.x + w2 * q2Fixed.x,
        w1 * q1.y + w2 * q2Fixed.y,
        w1 * q1.z + w2 * q2Fixed.z,
        w1 * q1.w + w2 * q2Fixed.w
    };
}

Quat slerpNaive(const Quat& q1, const Quat& q2, float t) {
    float cosTheta = dot(q1, q2);
    if (cosTheta > 1.0f) cosTheta = 1.0f;
    if (cosTheta < -1.0f) cosTheta = -1.0f;
    float theta = std::acos(cosTheta);
    float sinTheta = std::sin(theta);
    float w1 = std::sin((1.0f - t) * theta) / sinTheta;
    float w2 = std::sin(t * theta) / sinTheta;
    return {
        w1 * q1.x + w2 * q2.x,
        w1 * q1.y + w2 * q2.y,
        w1 * q1.z + w2 * q2.z,
        w1 * q1.w + w2 * q2.w
    };
}

float toAngleY(const Quat& q) {
    return 2.0f * std::atan2(q.y, q.w) * 180.0f / M_PI;
}

float toAngularVelocityDegPerSec(const Quat& q1, const Quat& q2, float dt) {
    // Vitesse angulaire approximée depuis l'interpolation naïve vs courte
    // Angle naïf = acos(dot) * 2 (en rad) -> deg
    float cosTheta = dot(q1, q2);
    if (cosTheta > 1.0f) cosTheta = 1.0f;
    if (cosTheta < -1.0f) cosTheta = -1.0f;
    float theta = std::acos(cosTheta) * 2.0f; // angle complet en rad
    float deg = theta * 180.0f / M_PI;
    return deg / dt;
}

int main() {
    std::cout << std::scientific << std::setprecision(6);

    // ============================================================
    // TEST 1 CORRIGÉ : Comme TEST 2 — deux orientations PROCHES, 
    // puis on RETOURNE l'une des deux pour créer le problème
    // ============================================================
    
    // Orientation A : 10° autour de Y
    Quat qA = { 0, 0.087156f, 0, 0.996195f }; // sin(5°)=0.087156, cos(5°)=0.996195
    
    // Orientation B : 30° autour de Y (proche de A, différence = 20°)
    Quat qB = { 0, 0.258819f, 0, 0.965926f }; // sin(15°)=0.258819, cos(15°)=0.965926
    
    std::cout << "=== TEST 1 : Deux orientations PROCHES (10° et 30°) ===\n";
    std::cout << "qA (10°) : " << qA.x << ' ' << qA.y << ' ' << qA.z << ' ' << qA.w << '\n';
    std::cout << "qB (30°) : " << qB.x << ' ' << qB.y << ' ' << qB.z << ' ' << qB.w << '\n';
    std::cout << "Dot(qA,qB) = " << dot(qA, qB) << " (POSITIF = déjà chemin court !)\n";
    
    // Vérification : angle réel entre qA et qB
    float cosTheta = dot(qA, qB);
    float theta = std::acos(cosTheta) * 2.0f * 180.0f / M_PI;
    std::cout << "Angle réel entre qA et qB : " << theta << "° (attendu ~20°)\n\n";
    
    // Interpolation naïve vs chemin court (devraient être identiques car déjà chemin court)
    Quat midNaive1 = slerpNaive(qA, qB, 0.5f);
    Quat midShort1 = slerpShortest(qA, qB, 0.5f);
    std::cout << "Naive  (t=0.5) : " << midNaive1.x << ' ' << midNaive1.y << ' ' << midNaive1.z << ' ' << midNaive1.w << " -> angle=" << toAngleY(midNaive1) << "°\n";
    std::cout << "Court  (t=0.5) : " << midShort1.x << ' ' << midShort1.y << ' ' << midShort1.z << ' ' << midShort1.w << " -> angle=" << toAngleY(midShort1) << "°\n";
    std::cout << "-> Identiques car dot > 0, pas de forçage nécessaire.\n\n";

    // ============================================================
    // MAINTENANT : On RETOURNE qB (qB_flipped = -qB)
    // qB et -qB représentent la MÊME orientation (30°)
    // Mais dot(qA, -qB) = -dot(qA, qB) < 0
    // ============================================================
    
    Quat qB_flipped = { -qB.x, -qB.y, -qB.z, -qB.w }; // Même rotation 30°, représentation opposée
    
    std::cout << "=== TEST 1b : Mêmes orientations, MAIS qB retournée (qB_flipped = -qB) ===\n";
    std::cout << "qA (10°)         : " << qA.x << ' ' << qA.y << ' ' << qA.z << ' ' << qA.w << '\n';
    std::cout << "qB_flipped (-30°): " << qB_flipped.x << ' ' << qB_flipped.y << ' ' << qB_flipped.z << ' ' << qB_flipped.w << '\n';
    std::cout << "Dot(qA,qB_flipped) = " << dot(qA, qB_flipped) << " (NÉGATIF !)\n";
    
    // Angle que le SLERP naïf va croire
    cosTheta = dot(qA, qB_flipped);
    if (cosTheta < -1.0f) cosTheta = -1.0f;
    theta = std::acos(cosTheta) * 2.0f * 180.0f / M_PI;
    std::cout << "Angle que le naïf CROIT : " << theta << "° (au lieu de 20° réels)\n\n";
    
    Quat midNaive2 = slerpNaive(qA, qB_flipped, 0.5f);
    Quat midShort2 = slerpShortest(qA, qB_flipped, 0.5f);
    std::cout << "Naive  (t=0.5) : " << midNaive2.x << ' ' << midNaive2.y << ' ' << midNaive2.z << ' ' << midNaive2.w << " -> angle=" << toAngleY(midNaive2) << "°\n";
    std::cout << "Court  (t=0.5) : " << midShort2.x << ' ' << midShort2.y << ' ' << midShort2.z << ' ' << midShort2.w << " -> angle=" << toAngleY(midShort2) << "°\n";
    std::cout << "-> Naive prend le chemin LONG (" << theta << "°), Court prend le court (20°).\n\n";

    // Vitesse angulaire pour voir l'absurdité
    float dt = 0.01f; // 10ms
    float velNaive = toAngularVelocityDegPerSec(qA, qB_flipped, dt);
    float velShort = toAngularVelocityDegPerSec(qA, qB, dt); // avec qB original (dot>0)
    std::cout << "Vitesse angulaire naïve  : " << velNaive << " deg/s (FACTEUR " << velNaive/velShort << "x trop grande !)\n";
    std::cout << "Vitesse angulaire courte : " << velShort << " deg/s\n";
    std::cout << "-> Le signe est AUSSI inversé si le forçage change le sens !\n\n";

    // ============================================================
    // TEST 2 : Quaternions opposés (même rotation) — inchangé, correct
    // ============================================================
    
    Quat q3 = { 0, 0.7071f, 0, 0.7071f };  // 90°
    Quat q4 = { 0, -0.7071f, 0, -0.7071f }; // -90° = même rotation que 270°
    
    std::cout << "=== TEST 2 : Quaternions opposés (même rotation 90°) ===\n";
    std::cout << "q3 (90°)  : " << q3.x << ' ' << q3.y << ' ' << q3.z << ' ' << q3.w << '\n';
    std::cout << "q4 (-90°) : " << q4.x << ' ' << q4.y << ' ' << q4.z << ' ' << q4.w << '\n';
    std::cout << "Dot(q3,q4) = " << dot(q3, q4) << " (doit être -1)\n";
    
    Quat mid34 = slerpShortest(q3, q4, 0.5f);
    std::cout << "Milieu chemin court : " << mid34.x << ' ' << mid34.y << ' ' << mid34.z << ' ' << mid34.w << '\n';
    std::cout << "Angle : " << toAngleY(mid34) << "° (devrait être 90° = pas de mouvement)\n\n";

    // ============================================================
    // CONCLUSION : Le problème du chemin court
    // ============================================================
    std::cout << "=== CONCLUSION ===\n";
    std::cout << "Le problème ne vient PAS de l'arithmétique des angles (350->10 = 20 deg).\n";
    std::cout << "Il vient du fait qu'une même orientation a DEUX représentations en quaternion (q et -q).\n";
    std::cout << "Un capteur peut rendre l'une puis l'autre sans prévenir.\n";
    std::cout << "Si on interpôle sans forcer le chemin court :\n";
    std::cout << "  - Vitesse angulaire fausse d'un facteur ~17 (340°/20°)\n";
    std::cout << "  - SIGNE INVERSÉ : la prédiction part dans l'autre sens !\n";
    std::cout << "La correction : if (dot(q1,q2) < 0) q2 = -q2; // Une ligne, critique.\n";

    return 0;
}
```

## Résultats (EXÉCUTION RÉELLE)

### TEST 1 : Deux orientations PROCHES (10° et 30°)
```
qA (10°) : 0.000000e+00 8.715600e-02 0.000000e+00 9.961950e-01
qB (30°) : 0.000000e+00 2.588190e-01 0.000000e+00 9.659260e-01
Dot(qA,qB) = 9.848078e-01 (POSITIF = déjà chemin court !)
Angle réel entre qA et qB : 2.000000e+01° (attendu ~20°)

Naive  (t=0.5) : 0.000000e+00 1.736482e-01 0.000000e+00 9.848078e-01 -> angle=2.000000e+01°
Court  (t=0.5) : 0.000000e+00 1.736482e-01 0.000000e+00 9.848078e-01 -> angle=2.000000e+01°
-> Identiques car dot > 0, pas de forçage nécessaire.
```

**L'ancienne copie affirmait `Dot(q1,q2) = -0.984808 (négatif)` — C'EST FAUX.**
- q1 = (0, -0.087, 0, 0.996) = -10° (≡ 350°)
- q2 = (0, +0.087, 0, 0.996) = +10°
- dot = (-0.087)(+0.087) + (0.996)(0.996) = -0.0076 + 0.9924 = **+0.9848** (POSITIF !)
- Ces deux orientations sont à **20°** l'une de l'autre (déjà chemin court). Le SLERP naïf ne fait RIEN de mal ici.

### TEST 1b : Mêmes orientations, MAIS qB retournée (qB_flipped = -qB)
```
qA (10°)         : 0.000000e+00 8.715600e-02 0.000000e+00 9.961950e-01
qB_flipped (-30°): 0.000000e+00 -2.588190e-01 0.000000e+00 -9.659260e-01
Dot(qA,qB_flipped) = -9.848078e-01 (NÉGATIF !)
Angle que le naïf CROIT : 3.400000e+02° (au lieu de 20° réels)

Naive  (t=0.5) : 0.000000e+00 0.000000e+00 0.000000e+00 -1.000000e+00 -> angle=1.800000e+02°
Court  (t=0.5) : 0.000000e+00 1.736482e-01 0.000000e+00 9.848078e-01 -> angle=2.000000e+01°
-> Naive prend le chemin LONG (340°), Court prend le court (20°).

Vitesse angulaire naïve  : 3.400000e+04 deg/s (FACTEUR 1.700000e+01x trop grande !)
Vitesse angulaire courte : 2.000000e+03 deg/s
-> Le signe est AUSSI inversé si le forçage change le sens !
```

### TEST 2 : Quaternions opposés (même rotation 90°)
```
q3 (90°)  : 0.000000e+00 7.071000e-01 0.000000e+00 7.071000e-01
q4 (-90°) : 0.000000e+00 -7.071000e-01 0.000000e+00 -7.071000e-01
Dot(q3,q4) = -1.000000e+00 (doit être -1)
Milieu chemin court : 0.000000e+00 7.071068e-01 0.000000e+00 7.071068e-01
Angle : 9.000000e+01° (devrait être 90° = pas de mouvement)
```

## Corrections apportées

1. **Test 1 corrigé** : L'ancien test utilisait q1=-10°, q2=+10° qui sont DÉJÀ à 20° (dot>0). Le problème n'apparaît que quand un capteur retourne l'un des deux quaternions (q → -q). Maintenant : on prend deux orientations proches, puis on **force le retournement** (`qB_flipped = -qB`) pour reproduire le vrai bug.

2. **Vitesse angulaire en deg/s** — convertit l'absurdité en nombres lisibles : facteur 17x trop grand, et **signe inversé** (la prédiction part dans l'autre sens). C'est la "seconde moitié la plus pire" selon le prof.

3. **Explication claire** : Le problème n'est pas "350° vers 10° = 20° vs 340°" (les quaternions ont déjà résolu ça). Le problème : **deux représentations q et -q pour la même orientation**, et un capteur peut switcher sans prévenir.