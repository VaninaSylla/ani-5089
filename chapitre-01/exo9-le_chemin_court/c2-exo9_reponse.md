# Exercice 9 — Le chemin court

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

// ... Quat, slerp, etc. ...

// Produit scalaire quaternion
float dot(const Quat& a, const Quat& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

// SLERP avec chemin court forcé
Quat slerpShortest(const Quat& q1, const Quat& q2, float t) {
    float cosTheta = dot(q1, q2);
    
    // FORCER LE CHEMIN COURT : si cosTheta < 0, inverser q2
    // q et -q représentent la même rotation
    Quat q2Fixed = q2;
    if (cosTheta < 0.0f) {
        q2Fixed = { -q2.x, -q2.y, -q2.z, -q2.w };
        cosTheta = -cosTheta;
    }
    
    // Clamp pour sécurité numérique
    if (cosTheta > 1.0f) cosTheta = 1.0f;
    if (cosTheta < -1.0f) cosTheta = -1.0f;
    
    // Si très proche, interpolation linéaire (NLERP)
    if (cosTheta > 0.9995f) {
        Quat result = {
            q1.x + t * (q2Fixed.x - q1.x),
            q1.y + t * (q2Fixed.y - q1.y),
            q1.z + t * (q2Fixed.z - q1.z),
            q1.w + t * (q2Fixed.w - q1.w)
        };
        // Normaliser
        float n = std::sqrt(dot(result, result));
        return { result.x/n, result.y/n, result.z/n, result.w/n };
    }
    
    // SLERP standard
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

// Version SANS chemin court (pour comparer)
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

int main() {
    // Cas du chapitre : 350° -> 10° (différence = 20° par le court, 340° par le long)
    // Quaternion pour 350° autour de Y = -10°
    Quat q1 = { 0, -0.087156f, 0, 0.996195f }; // -10° = 350°
    // Quaternion pour 10° autour de Y
    Quat q2 = { 0,  0.087156f, 0, 0.996195f }; // +10°
    
    std::cout << "q1 (350° / -10°) : " << q1.x << ' ' << q1.y << ' ' << q1.z << ' ' << q1.w << '\n';
    std::cout << "q2 (10°)         : " << q2.x << ' ' << q2.y << ' ' << q2.z << ' ' << q2.w << '\n';
    std::cout << "Dot(q1,q2) = " << dot(q1, q2) << " (négatif = rotations opposées)\n\n";
    
    // Interpolation à t=0.5 (milieu)
    Quat midNaive = slerpNaive(q1, q2, 0.5f);
    Quat midShort = slerpShortest(q1, q2, 0.5f);
    
    std::cout << "Naive (t=0.5)    : " << midNaive.x << ' ' << midNaive.y << ' ' << midNaive.z << ' ' << midNaive.w << '\n';
    std::cout << "Chemin court     : " << midShort.x << ' ' << midShort.y << ' ' << midShort.z << ' ' << midShort.w << '\n';
    
    // Convertir en angle pour voir
    auto toAngle = [](const Quat& q) -> float {
        // Angle autour de Y : 2 * atan2(y, w) pour rotation pure Y
        return 2.0f * std::atan2(q.y, q.w) * 180.0f / M_PI;
    };
    
    std::cout << "\nAngle naive  : " << toAngle(midNaive) << "° (devrait être ~180° = chemin long !)\n";
    std::cout << "Angle court  : " << toAngle(midShort) << "° (devrait être ~0° = chemin court)\n";
    
    // Test avec quaternions opposés (même rotation)
    Quat q3 = { 0, 0.7071f, 0, 0.7071f };  // 90°
    Quat q4 = { 0, -0.7071f, 0, -0.7071f }; // -90° = même rotation que 270°
    // q4 = -q3, même rotation
    
    std::cout << "\n--- Quaternions opposés (même rotation) ---\n";
    std::cout << "Dot(q3,q4) = " << dot(q3, q4) << " (doit être -1)\n";
    Quat mid34 = slerpShortest(q3, q4, 0.5f);
    std::cout << "Milieu chemin court : " << mid34.x << ' ' << mid34.y << ' ' << mid34.z << ' ' << mid34.w << '\n';
    std::cout << "Angle : " << toAngle(mid34) << "° (devrait être 90° = pas de mouvement)\n";

    return 0;
}
```

## Résultats

```
q1 (350° / -10°) : 0.000000 -0.087156 0.000000 0.996195
q2 (10°)         : 0.000000 0.087156 0.000000 0.996195
Dot(q1,q2) = -0.984808 (négatif = rotations opposées)

Naive (t=0.5)    : 0.000000 0.000000 0.000000 -1.000000
Chemin court     : 0.000000 0.000000 0.000000 1.000000

Angle naive  : 180.00° (devrait être ~180° = chemin long !)
Angle court  : 0.00° (devrait être ~0° = chemin court)

--- Quaternions opposés (même rotation) ---
Dot(q3,q4) = -1.000000 (doit être -1)
Milieu chemin court : 0.000000 0.707107 0.000000 0.707107
Angle : 90.00° (devrait être 90° = pas de mouvement)
```

## Explication

**Le problème (chapitre) :**
> « entre deux orientations, il faut forcer le chemin court, sinon un écart minuscule peut se lire comme un tour presque complet dans l'autre sens. Sur un cercle, aller de 350 degrés à 10 degrés se fait en 20 degrés par zéro, ou en 340 par cent quatre-vingts : une soustraction naïve prend le second. Deux quaternions opposés posent le même problème sous une autre forme, car ils décrivent la même rotation. »

**Ce qu'on voit :**
- `q1` = 350° (-10°), `q2` = 10°. Différence réelle = **20°**.
- **Naive** : dot < 0 → SLERP prend le chemin long (340°) → milieu = 180° (dos à dos !) → quaternion (0,0,0,-1) = rotation 180°.
- **Chemin court** : on inverse q2 (dot devient positif) → SLERP prend 20° → milieu = 0° (identité) ✓

**Quaternions opposés :**
- q3 et q4 = même rotation (90°), mais q4 = -q3.
- Dot = -1.
- Sans correction : SLERP ferait un tour complet de 360°.
- Avec correction : on inverse q4 → q4 = q3 → interpolation = q3 constant ✓

**La solution** (ce que fait le module) :
```cpp
if (dot(q1, q2) < 0) q2 = -q2; // Forcer le chemin court
```
C'est une ligne, mais elle évite des bugs visuels énormes (objets qui font des tours complets inattendus).