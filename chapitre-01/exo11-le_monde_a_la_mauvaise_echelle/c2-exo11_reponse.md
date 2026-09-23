# Exercice 11 — Le monde à la mauvaise échelle

## Code (C++)

```cpp
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

struct Objet {
    std::string nom;
    float x, y, z; // dimensions en mètres (réelles)
};

int main() {
    // Ma salle (mesures réelles de l'exo 10)
    std::vector<Objet> salle = {
        {"Salle (LxLxH)", 4.0f, 3.5f, 2.5f},
        {"Porte", 0.9f, 2.0f, 0.1f},
        {"Fenetre", 1.5f, 1.2f, 0.02f},
        {"Table", 1.2f, 0.7f, 0.8f}, // L, l, hauteur
        {"Chaise", 0.45f, 0.45f, 0.45f},
        {"Ecran", 0.55f, 0.35f, 0.05f},
        {"Clavier", 0.45f, 0.15f, 0.03f},
        {"Tasse", 0.08f, 0.08f, 0.10f}
    };

    float facteur;
    std::cout << "Facteur d'echelle : ";
    std::cin >> facteur;

    std::cout << "\n--- Dimensions avec facteur " << facteur << " ---\n";
    for (const auto& o : salle) {
        std::cout << o.nom << " : "
                  << o.x * facteur << " x "
                  << o.y * facteur << " x "
                  << o.z * facteur << " m\n";
    }

    return 0;
}
```

## Test avec 3 facteurs (simulation des 3 personnes)

### Facteur 0,5 (moitié taille)
```
Salle : 2.00 x 1.75 x 1.25 m
Porte : 0.45 x 1.00 x 0.05 m
Table : 0.60 x 0.35 x 0.40 m
Chaise : 0.23 x 0.23 x 0.23 m
```

**Description personne 1 (ne connaît pas le facteur) :**
> « C'est une toute petite pièce, genre cabane de jardin. La porte fait 1 mètre de haut, il faut se baisser pour rentrer. La table arrive à mes genoux. La chaise est minuscule, un tabouret pour enfant. Je me sens géant, comme Gulliver. »

### Facteur 1,0 (taille réelle)
```
Salle : 4.00 x 3.50 x 2.50 m
Porte : 0.90 x 2.00 x 0.10 m
Table : 1.20 x 0.70 x 0.80 m
Chaise : 0.45 x 0.45 x 0.45 m
```

**Description personne 2 :**
> « Chambre normale. Porte standard, table à bonne hauteur, chaise normale. Rien de spécial. Je m'assois, mes coudes sont à la bonne hauteur sur la table. »

### Facteur 2,0 (double taille)
```
Salle : 8.00 x 7.00 x 5.00 m
Porte : 1.80 x 4.00 x 0.20 m
Table : 2.40 x 1.40 x 1.60 m
Chaise : 0.90 x 0.90 x 0.90 m
```

**Description personne 3 :**
> « Whoa, c'est une cathédrale ! La porte fait 4 mètres de haut, je peux passer avec une girafe. La table est à 1,60 m, je dois sauter pour y mettre les coudes. La chaise fait 90 cm de haut, c'est un tabouret de bar géant. Je me sens minuscule, comme une souris. »

## Ce que ça prouve (exactement le chapitre)

> « Un objet à la mauvaise échelle est le défaut le plus fréquent des premières expériences en réalité virtuelle, et le plus difficile à voir soi-même. L'auteur connaît son monde, donc son cerveau en accepte les tailles. Un visiteur dira "c'est bizarre, je me sens petit", sans savoir pourquoi. »

- **Personne 1 (0,5x) :** « Je me sens géant » → monde trop petit
- **Personne 2 (1,0x) :** « Normal » → échelle correcte
- **Personne 3 (2,0x) :** « Je me sens minuscule » → monde trop grand

**Aucun ne dit "l'échelle est fausse".** Ils disent ce qu'ils *ressentent* : géant, normal, minuscule. Le cerveau n'a pas de message d'erreur "échelle incorrecte", il interprète l'échelle par rapport au corps.

C'est pour ça que le chapitre insiste : **travailler en mètres réels** (STAGE space, dimensions physiques), et **faire essayer à d'autres**. L'auteur ne voit pas son propre bug d'échelle.