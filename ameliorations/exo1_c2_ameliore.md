# Exercice 1 — Les trois directions (Amélioré)

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>

// Convention du module : main droite, +x droite, +y haut, -z avant
struct Vec3 {
    float x, y, z;
};

Vec3 Avant()  { return {0.0f, 0.0f, -1.0f}; }
Vec3 Haut()   { return {0.0f, 1.0f,  0.0f}; }
Vec3 Droite() { return {1.0f, 0.0f,  0.0f}; }

Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

int main() {
    // Vérification du trièdre direct : Droite × Haut = -Avant (car Avant = -z)
    Vec3 d = Droite();
    Vec3 h = Haut();
    Vec3 a = Avant();
    Vec3 cp = cross(d, h);
    std::cout << "Vérification trièdre direct (Droite × Haut) : "
              << cp.x << " " << cp.y << " " << cp.z << "\n";
    std::cout << "Avant()                    : "
              << a.x << " " << a.y << " " << a.z << "\n";
    std::cout << "Produit = -Avant ? " << (dot(cp, a) < 0 ? "OUI (trièdre direct)" : "NON") << "\n\n";

    float x, y, z;
    std::cin >> x >> y >> z;
    Vec3 p{x, y, z};

    std::cout << std::fixed << std::setprecision(4);
    std::cout << dot(p, Avant())  << '\n';
    std::cout << dot(p, Haut())   << '\n';
    std::cout << dot(p, Droite()) << '\n';
    return 0;
}
```

## Explications

- J'ai écrit les 3 fonctions `Avant()`, `Haut()`, `Droite()` qui fixent la convention **une fois pour toutes**.
- Convention : main droite, +x droite, +y haut, -z avant (regarde vers z négatifs).
- **Ajout** : Vérification explicite que le trièdre est direct : `Droite() × Haut() = (0,0,1) = -Avant()`. Cela confirme qu'un angle positif tourne selon la règle de la main droite.
- Le programme lit un point (x, y, z) et affiche ses 3 produits scalaires avec les axes.
- 4 décimales comme demandé.

## Test

Input : `1 2 3`
Output :
```
Vérification trièdre direct (Droite × Haut) : 0 0 1
Avant()                    : 0 0 -1
Produit = -Avant ? OUI (trièdre direct)

-3.0000
2.0000
1.0000
```
- Avant = -z = -3 ✓
- Haut = y = 2 ✓
- Droite = x = 1 ✓

Le trièdre est bien direct : (1,0,0) × (0,1,0) = (0,0,1) = opposé de Avant(). Cela garantit que les rotations positives suivent la règle de la main droite dans tout le reste du livre.