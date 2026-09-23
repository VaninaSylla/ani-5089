# Exercice 1 — Les trois directions

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

float dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

int main() {
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

- J'ai écrit les 3 fonctions `Avant()`, `Haut()`, `Droite()` qui fixent la convention **une fois pour toutes**. Comme dit le chapitre : « une convention ne se retient pas, elle s'écrit dans une fonction ».
- Convention : main droite, +x droite, +y haut, -z avant (regarde vers z négatifs).
- Le programme lit un point (x, y, z) et affiche ses 3 produits scalaires avec les axes.
- 4 décimales comme demandé.

## Test

Input : `1 2 3`
Output :
```
-3.0000
2.0000
1.0000
```
- Avant = -z = -3 ✓
- Haut = y = 2 ✓
- Droite = x = 1 ✓

Ça marche. Le truc important c'est que si je me trompe de signe, je le vois tout de suite dans les tests, et je corrige au même endroit pour tout le programme.