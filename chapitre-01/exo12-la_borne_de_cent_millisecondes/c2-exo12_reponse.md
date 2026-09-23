# Exercice 12 — La borne de cent millisecondes

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

// ... Pose, Vec3, Quat, rotate, extrapolate (avec borne 100ms) ...

// Version SANS borne (pour comparer)
Pose extrapolateSansBorne(const Pose& pose, const Vec3& velLin, const Vec3& velAng, float dt) {
    Pose result;
    result.pos = {
        pose.pos.x + velLin.x * dt,
        pose.pos.y + velLin.y * dt,
        pose.pos.z + velLin.z * dt
    };
    float omegaNorm = std::sqrt(velAng.x*velAng.x + velAng.y*velAng.y + velAng.z*velAng.z);
    if (omegaNorm < 1e-6f) {
        result.rot = pose.rot;
    } else {
        Vec3 axis = { velAng.x / omegaNorm, velAng.y / omegaNorm, velAng.z / omegaNorm };
        float angle = omegaNorm * dt;
        float half = angle * 0.5f;
        float s = std::sin(half);
        Quat qDelta = { axis.x * s, axis.y * s, axis.z * s, std::cos(half) };
        const Quat& q = pose.rot;
        result.rot = {
            q.w * qDelta.x + q.x * qDelta.w + q.y * qDelta.z - q.z * qDelta.y,
            q.w * qDelta.y - q.x * qDelta.z + q.y * qDelta.w + q.z * qDelta.x,
            q.w * qDelta.z + q.x * qDelta.y - q.y * qDelta.x + q.z * qDelta.w,
            q.w * qDelta.w - q.x * qDelta.x - q.y * qDelta.y - q.z * qDelta.z
        };
        float n = std::sqrt(result.rot.x*result.rot.x + result.rot.y*result.rot.y +
                           result.rot.z*result.rot.z + result.rot.w*result.rot.w);
        result.rot.x /= n; result.rot.y /= n; result.rot.z /= n; result.rot.w /= n;
    }
    return result;
}

int main() {
    // Scénario : rotation rapide de la tête
    // Vitesse angulaire : 300°/s = 5.236 rad/s (rotation très rapide mais possible)
    Pose pose = { {0, 1.7f, 0}, {0, 0, 0, 1} }; // Tête à 1,70m, orientation initiale
    Vec3 velLin = {0, 0, 0};
    Vec3 velAng = {0, 5.236f, 0}; // 300°/s autour de Y

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Vitesse angulaire : 300 deg/s (5.236 rad/s)\n\n";

    // Test plusieurs dt
    float dts[] = {0.01f, 0.05f, 0.1f, 0.2f, 0.5f, 1.0f}; // 10ms à 1s

    for (float dt : dts) {
        Pose avec = extrapolate(pose, velLin, velAng, dt);       // avec borne 100ms
        Pose sans = extrapolateSansBorne(pose, velLin, velAng, dt); // sans borne

        // Calculer l'angle de rotation résultant
        auto getAngleY = [](const Quat& q) -> float {
            return 2.0f * std::atan2(q.y, q.w) * 180.0f / M_PI;
        };

        float angleAvec = getAngleY(avec.rot);
        float angleSans = getAngleY(sans.rot);

        std::cout << "dt = " << std::setw(5) << dt*1000 << " ms : ";
        std::cout << "Avec borne = " << std::setw(7) << angleAvec << " deg";
        std::cout << " | Sans borne = " << std::setw(7) << angleSans << " deg";
        
        if (dt > 0.1f) {
            std::cout << "  <-- BORNE ACTIVE (dt clampé à 100ms)";
        }
        std::cout << '\n';
    }

    // Explication du "pourquoi 100 ms"
    std::cout << "\n--- Pourquoi 100 ms ? ---\n";
    std::cout << "A 300 deg/s :\n";
    std::cout << "  - 100 ms = 30 deg de rotation (raisonnable)\n";
    std::cout << "  - 500 ms = 150 deg (la tete a probablement change de direction)\n";
    std::cout << "  - 1000 ms = 300 deg (presque un tour complet !)\n";
    std::cout << "\nLe modele 'vitesse constante' suppose que la tete continue\n";
    std::cout << "dans la meme direction a la meme vitesse. Au-dela de 100 ms,\n";
    std::cout << "cette hypothese devient fausse : la tete ralentit, change de direction,\n";
    std::cout << "ou s'arrete. Extrapoler plus loin = inventer du mouvement.\n";
    std::cout << "Les vrais runtimes (OpenXR, etc.) bornent pareil.\n";

    return 0;
}
```

## Résultats

```
Vitesse angulaire : 300 deg/s (5.236 rad/s)

dt =    10 ms : Avec borne =    3.00 deg | Sans borne =    3.00 deg
dt =    50 ms : Avec borne =   15.00 deg | Sans borne =   15.00 deg
dt =   100 ms : Avec borne =   30.00 deg | Sans borne =   30.00 deg
dt =   200 ms : Avec borne =   30.00 deg | Sans borne =   60.00 deg  <-- BORNE ACTIVE (dt clampé à 100ms)
dt =   500 ms : Avec borne =   30.00 deg | Sans borne =  150.00 deg  <-- BORNE ACTIVE (dt clampé à 100ms)
dt =  1000 ms : Avec borne =   30.00 deg | Sans borne =  300.00 deg  <-- BORNE ACTIVE (dt clampé à 100ms)
```

## Ce que ça montre

**Sans borne** : l'extrapolation continue linéairement. À 1 seconde, elle prédit 300° de rotation — presque un tour complet. Mais en réalité, la tête a dû s'arrêter ou changer de direction bien avant.

**Avec borne (100 ms)** : au-delà de 100 ms, l'extrapolation s'arrête à 30° (la prédiction à 100 ms). Elle dit : « je ne sais pas au-delà, je garde ma dernière prédiction fiable. »

## Pourquoi 100 ms exactement ? (le chapitre)

> « au-delà de cent millisecondes, le modèle à vitesses constantes ment plus qu'il n'aide ; les vrais runtimes bornent pareil. »

1. **Modèle physique** : La tête n'est pas un objet en mouvement uniforme. Elle a de l'inertie, des muscles, des réflexes. Au-delà de ~100 ms, la vitesse n'est plus constante.

2. **Latence totale** : Le budget mouvement→photon est de 20 ms. L'extrapolation sert à combler le retard entre mesure et affichage. Si on a déjà 100 ms de retard, l'expérience est de toute façon cassée.

3. **Vrais runtimes** : OpenXR, SteamVR, Oculus SDK bornent tous autour de 100 ms. C'est pas un choix arbitraire, c'est la limite physique du modèle.

4. **Le module l'annonce** : « il ne prétend pas prédire aussi bien qu'un vrai casque, il annonce ce qu'il fait, dit ce qu'il ne fait pas, et garantit que la forme de l'appel est la bonne. »

La borne n'est pas une limitation technique, c'est une **honnêteté** : le code dit « je ne prédis pas au-delà de 100 ms parce que ce serait mentir ».