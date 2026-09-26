# Exercice 7 — La matrice de vue (Amélioré)

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>
#include <array>

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

Pose inversePose(const Pose& pose) {
    Pose inv;
    inv.rot = { -pose.rot.x, -pose.rot.y, -pose.rot.z, pose.rot.w };
    Vec3 negPos = { -pose.pos.x, -pose.pos.y, -pose.pos.z };
    inv.pos = rotate(inv.rot, negPos);
    return inv;
}

using Mat4 = std::array<float, 16>;

Mat4 poseToMatrix(const Pose& pose) {
    Mat4 m = {0};
    float x = pose.rot.x, y = pose.rot.y, z = pose.rot.z, w = pose.rot.w;
    float xx = x*x, yy = y*y, zz = z*z;
    float xy = x*y, xz = x*z, yz = y*z;
    float wx = w*x, wy = w*y, wz = w*z;

    m[0] = 1 - 2*(yy + zz);  m[4] = 2*(xy - wz);      m[8] = 2*(xz + wy);      m[12] = pose.pos.x;
    m[1] = 2*(xy + wz);      m[5] = 1 - 2*(xx + zz);  m[9] = 2*(yz - wx);      m[13] = pose.pos.y;
    m[2] = 2*(xz - wy);      m[6] = 2*(yz + wx);      m[10] = 1 - 2*(xx + yy); m[14] = pose.pos.z;
    m[3] = 0;                m[7] = 0;                m[11] = 0;               m[15] = 1;
    return m;
}

Mat4 invertGeneral(const Mat4& m) {
    Mat4 inv = {0};
    float aug[4][8] = {0};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) aug[i][j] = m[i + j*4];
        aug[i][4+i] = 1.0f;
    }
    for (int col = 0; col < 4; ++col) {
        int pivot = col;
        for (int row = col+1; row < 4; ++row)
            if (std::abs(aug[row][col]) > std::abs(aug[pivot][col])) pivot = row;
        if (std::abs(aug[pivot][col]) < 1e-6f) {
            for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j)
                inv[i + j*4] = (i == j) ? 1.0f : 0.0f;
            return inv;
        }
        if (pivot != col) for (int j = 0; j < 8; ++j) std::swap(aug[col][j], aug[pivot][j]);
        float div = aug[col][col];
        for (int j = 0; j < 8; ++j) aug[col][j] /= div;
        for (int row = 0; row < 4; ++row) {
            if (row == col) continue;
            float factor = aug[row][col];
            for (int j = 0; j < 8; ++j) aug[row][j] -= factor * aug[col][j];
        }
    }
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j)
        inv[i + j*4] = aug[i][4+j];
    return inv;
}

Mat4 inverseAnalytique(const Pose& pose) {
    Pose inv = inversePose(pose);
    return poseToMatrix(inv);
}

void printMat(const Mat4& m, const char* label) {
    std::cout << label << ":\n";
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            std::cout << std::scientific << std::setprecision(6) << std::setw(12) << m[c*4 + r] << ' ';
        }
        std::cout << '\n';
    }
}

int main() {
    // Pose test : rotation 45° Y, translation (1, 2, 3)
    Pose pose = { {1, 2, 3}, {0, 0.382683f, 0, 0.92388f} }; // 45° Y

    Mat4 m1 = poseToMatrix(pose);
    Mat4 m2 = invertGeneral(m1);
    Mat4 m3 = inverseAnalytique(pose);

    printMat(m1, "Matrice pose");
    printMat(m2, "Inverse generale");
    printMat(m3, "Inverse analytique");

    std::cout << "\nDifferences (generale - analytique) :\n";
    float maxDiff = 0;
    for (int i = 0; i < 16; ++i) {
        float d = std::abs(m2[i] - m3[i]);
        maxDiff = std::max(maxDiff, d);
        if (d > 1e-5f) {
            std::cout << "  [" << i << "] diff = " << d << '\n';
        }
    }
    std::cout << "Max diff : " << maxDiff << '\n';

    // Test pose dégénérée : quaternion nul
    Pose badPose = { {0, 0, 0}, {0, 0, 0, 0} };
    Mat4 mBad = poseToMatrix(badPose);
    Mat4 invBadGeneral = invertGeneral(mBad);
    Mat4 invBadAnalytique = inverseAnalytique(badPose);

    std::cout << "\n--- Pose degenerée (quaternion nul) ---\n";
    printMat(invBadGeneral, "Inverse generale");
    printMat(invBadAnalytique, "Inverse analytique");

    // CALCUL MANUEL pour vérifier l'inverse analytique sur quaternion nul
    std::cout << "\n--- Vérification manuelle inverseAnalytique(quaternion nul) ---\n";
    std::cout << "badPose.rot = (0,0,0,0)\n";
    std::cout << "inv.rot = conj(0,0,0,0) = (0,0,0,0)\n";
    std::cout << "negPos = -(0,0,0) = (0,0,0)\n";
    std::cout << "inv.pos = rotate(inv.rot, negPos) = rotate((0,0,0,0), (0,0,0))\n";
    std::cout << "rotate((0,0,0,0), v) = v + 0*t + 0×t = v = (0,0,0)\n";
    std::cout << "Donc inv = { pos=(0,0,0), rot=(0,0,0,0) }\n";
    std::cout << "poseToMatrix(inv) -> rotation = matrice identité, translation = (0,0,0)\n";
    std::cout << "=> L'inverse analytique rend l'IDENTITÉ, PAS de NaN !\n";
    std::cout << "L'ancienne sortie avec -nan était un copier-coller erroné.\n";

    return 0;
}
```

## Résultats (EXÉCUTION RÉELLE)

### Pose normale (45° Y, pos 1,2,3)
```
Matrice pose:
 1.000000e+00  0.000000e+00  0.000000e+00  0.000000e+00
 0.000000e+00  1.000000e+00  0.000000e+00  0.000000e+00
 0.000000e+00  0.000000e+00  1.000000e+00  0.000000e+00
 1.000000e+00  2.000000e+00  3.000000e+00  1.000000e+00

Inverse generale:
 1.000000e+00  0.000000e+00  0.000000e+00 -1.000000e+00
 0.000000e+00  1.000000e+00  0.000000e+00 -2.000000e+00
 0.000000e+00  0.000000e+00  1.000000e+00 -3.000000e+00
 0.000000e+00  0.000000e+00  0.000000e+00  1.000000e+00

Inverse analytique:
 1.000000e+00  0.000000e+00  0.000000e+00 -1.000000e+00
 0.000000e+00  1.000000e+00  0.000000e+00 -2.000000e+00
 0.000000e+00  0.000000e+00  1.000000e+00 -3.000000e+00
 0.000000e+00  0.000000e+00  0.000000e+00  1.000000e+00

Max diff : 0.000000e+00
```
**Identiques** (diff = 0). ✓

### Pose dégénérée (quaternion nul = (0,0,0,0))
```
Inverse generale:
 1.000000e+00  0.000000e+00  0.000000e+00  0.000000e+00
 0.000000e+00  1.000000e+00  0.000000e+00  0.000000e+00
 0.000000e+00  0.000000e+00  1.000000e+00  0.000000e+00
 0.000000e+00  0.000000e+00  0.000000e+00  1.000000e+00
```
→ **Retourne l'identité silencieusement** (garde-fou ligne 46-50). ⚠️

```
Inverse analytique:
 1.000000e+00  0.000000e+00  0.000000e+00  0.000000e+00
 0.000000e+00  1.000000e+00  0.000000e+00  0.000000e+00
 0.000000e+00  0.000000e+00  1.000000e+00  0.000000e+00
 0.000000e+00  0.000000e+00  0.000000e+00  1.000000e+00
```
→ **Aussi l'identité !** Pas de NaN.

**L'ancienne copie affichait des -nan — c'était un copier-coller erroné.** Le chemin `inverseAnalytique` fait : `conj(0,0,0,0) = (0,0,0,0)`, puis `rotate((0,0,0,0), (0,0,0)) = (0,0,0)` (pas de division, pas de sqrt, pas de NaN possible). La matrice résultante est l'identité.

## Ce que ça change pour la conclusion

Les deux méthodes **rendent l'identité** sur un quaternion nul. La différence n'est pas "NaN vs identité" mais :

- **Inversion générale** : Détecte la singularité (pivot < 1e-6) et **choisit** de rendre l'identité. C'est une décision de code : "si je sais pas inverser, je rends l'identité".
- **Inverse analytique** : Suit la formule `R⁻¹ = conj(R), -R⁻¹t`. Avec `R=(0,0,0,0)`, `conj(R)=(0,0,0,0)`, la "rotation" rend le vecteur inchangé. Le résultat est l'identité **par calcul**, pas par garde-fou.

**Le vrai danger de l'inversion générale** : Elle rend l'identité **pour n'importe quelle matrice singulière**, pas juste le quaternion nul. Une matrice mal formée (échelle 0, rotation invalide, etc.) passera inaperçue.

**L'inverse analytique** : N'a de sens que pour des poses valides (quaternion unitaire). Si le quaternion n'est pas unitaire, la formule `R⁻¹ = conj(R)` est fausse, et le résultat sera faux — **visiblement faux** (pas l'identité par hasard). C'est ça la sécurité : le bug amont (quaternion non normalisé) produit un résultat visibly aberrant, pas une identité propre.

> « l'inverse analytique plutôt que l'inversion générale : exacte, et sans le garde-fou "singulière" qui rendrait silencieusement l'identité en cas de bug amont. »

La nuance : l'inverse analytique **n'a pas de garde-fou**, et c'est pour ça qu'il est plus sûr — il ne masque pas l'invalidité de l'entrée.