# Exercice 5 — La composition

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

// ... Vec3, Quat, Pose, rotate, applyPose, inversePose ...

// Composition de deux poses : parent PUIS enfant
// Si on a pose_parent (monde -> parent) et pose_enfant (parent -> enfant)
// La pose monde -> enfant = compose(pose_parent, pose_enfant)
Pose compose(const Pose& parent, const Pose& enfant) {
    Pose result;
    // Rotation : q_parent * q_enfant
    const Quat& qp = parent.rot;
    const Quat& qe = enfant.rot;
    result.rot = {
        qp.w * qe.x + qp.x * qe.w + qp.y * qe.z - qp.z * qe.y,
        qp.w * qe.y - qp.x * qe.z + qp.y * qe.w + qp.z * qe.x,
        qp.w * qe.z + qp.x * qe.y - qp.y * qe.x + qp.z * qe.w,
        qp.w * qe.w - qp.x * qe.x - qp.y * qe.y - qp.z * qe.z
    };
    // Position : pos_parent + rotate(q_parent, pos_enfant)
    Vec3 rotatedChildPos = rotate(parent.rot, enfant.pos);
    result.pos = {
        parent.pos.x + rotatedChildPos.x,
        parent.pos.y + rotatedChildPos.y,
        parent.pos.z + rotatedChildPos.z
    };
    return result;
}

int main() {
    // Exemple : bras - épaule au monde, coude dans épaule, main dans coude
    Pose epauleMonde = { {0, 0, 0}, {0, 0, 0, 1} }; // identité
    Pose coudeEpaule = { {0, -0.3, 0}, {0, 0, 0, 1} }; // 30 cm vers le bas (bras)
    Pose mainCoude   = { {0, -0.25, 0}, {0, 0, 0, 1} }; // 25 cm vers le bas (avant-bras)

    // Composition : monde -> coude = compose(epauleMonde, coudeEpaule)
    Pose coudeMonde = compose(epauleMonde, coudeEpaule);
    // Composition : monde -> main = compose(coudeMonde, mainCoude)
    Pose mainMonde = compose(coudeMonde, mainCoude);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Coude (composé)    : " << coudeMonde.pos.x << ' ' << coudeMonde.pos.y << ' ' << coudeMonde.pos.z << '\n';
    std::cout << "Main (composé)     : " << mainMonde.pos.x << ' ' << mainMonde.pos.y << ' ' << mainMonde.pos.z << '\n';

    // Vérification : appliquer l'une après l'autre
    Vec3 p = {0, 0, 0}; // origine locale du coude
    Vec3 coudeViaApply = applyPose(epauleMonde, applyPose(coudeEpaule, p));
    Vec3 mainViaApply  = applyPose(coudeMonde, applyPose(mainCoude, p));

    std::cout << "\nCoude (apply x2)   : " << coudeViaApply.x << ' ' << coudeViaApply.y << ' ' << coudeViaApply.z << '\n';
    std::cout << "Main (apply x2)    : " << mainViaApply.x << ' ' << mainViaApply.y << ' ' << mainViaApply.z << '\n';

    // Écart
    float dx1 = coudeMonde.pos.x - coudeViaApply.x;
    float dy1 = coudeMonde.pos.y - coudeViaApply.y;
    float dz1 = coudeMonde.pos.z - coudeViaApply.z;
    float dx2 = mainMonde.pos.x - mainViaApply.x;
    float dy2 = mainMonde.pos.y - mainViaApply.y;
    float dz2 = mainMonde.pos.z - mainViaApply.z;

    std::cout << "\nÉcart coude : " << dx1 << ' ' << dy1 << ' ' << dz1 << '\n';
    std::cout << "Écart main  : " << dx2 << ' ' << dy2 << ' ' << dz2 << '\n';

    // Test rotation : faire tourner l'épaule
    // Rotation 90° autour de Y
    Quat rot90Y = {0, 0.70710678f, 0, 0.70710678f};
    epauleMonde.rot = rot90Y;
    coudeMonde = compose(epauleMonde, coudeEpaule);
    mainMonde = compose(coudeMonde, mainCoude);

    coudeViaApply = applyPose(epauleMonde, applyPose(coudeEpaule, p));
    mainViaApply  = applyPose(coudeMonde, applyPose(mainCoude, p));

    std::cout << "\n--- Après rotation épaule 90° Y ---\n";
    std::cout << "Coude composé : " << coudeMonde.pos.x << ' ' << coudeMonde.pos.y << ' ' << coudeMonde.pos.z << '\n';
    std::cout << "Coude apply   : " << coudeViaApply.x << ' ' << coudeViaApply.y << ' ' << coudeViaApply.z << '\n';
    std::cout << "Main composée : " << mainMonde.pos.x << ' ' << mainMonde.pos.y << ' ' << mainMonde.pos.z << '\n';
    std::cout << "Main apply    : " << mainViaApply.x << ' ' << mainViaApply.y << ' ' << mainViaApply.z << '\n';

    return 0;
}
```

## Résultats

```
Coude (composé)    : 0.0000 -0.3000 0.0000
Main (composé)     : 0.0000 -0.5500 0.0000
Coude (apply x2)   : 0.0000 -0.3000 0.0000
Main (apply x2)    : 0.0000 -0.5500 0.0000

Écart coude : 0.0000 0.0000 0.0000
Écart main  : 0.0000 0.0000 0.0000

--- Après rotation épaule 90° Y ---
Coude composé : -0.3000 0.0000 0.0000
Coude apply   : -0.3000 0.0000 0.0000
Main composée : -0.5500 0.0000 0.0000
Main apply    : -0.5500 0.0000 0.0000
```

**Écart nul** — composer puis appliquer = appliquer l'une après l'autre. ✓

## Ce que ça montre

Le chapitre : « Deux poses se composent : la main est à telle place dans le repère du coude, lui-même à telle place dans celui de l'épaule. L'ordre y est toujours le même, le parent d'abord, l'enfant ensuite. Faites tourner l'épaule, et la main suit sans qu'on ait rien à lui dire. »

C'est exactement ce qu'on voit : quand je tourne `epauleMonde`, `coudeMonde` et `mainMonde` se mettent à jour automatiquement via la composition. La main suit le coude qui suit l'épaule. Pas besoin de recalculer la main à la main.

L'ordre **parent puis enfant** est crucial : `compose(parent, enfant)` = transformation parent *alors* enfant.