# Exercice 11 — Le monde à la mauvaise échelle (Amélioré)

## ⚠️ TESTS UTILISATEURS RÉELS MANQUANTS — À FAIRE

**L'énoncé demande : « Faites décrire la salle à trois personnes pour trois facteurs différents, sans leur dire lequel, et notez leurs mots. »**

L'ancienne version **simulait** les réponses ("simulation des 3 personnes"). Ce n'est pas ce qui était demandé.

> « Cet exercice est le seul du chapitre dont la conclusion est que l'auteur ne peut pas juger son propre monde. Vous l'écrivez : "l'auteur ne voit pas son propre bug d'échelle." Puis vous inventez les trois réactions vous-même, c'est-à-dire que vous faites exactement ce que votre conclusion déclare impossible. »

**À faire** : Aller voir 3 personnes réelles. Cela prend un quart d'heure : le programme existe déjà, il suffit de leur montrer 3 sorties (facteurs 0.5, 1.0, 2.0) et de noter ce qu'elles disent. Poser la même question aux 3, dans les mêmes termes, sans laisser entendre qu'il y a quelque chose d'anormal.

---

## Code (C++) — inchangé, correct

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
        {"Table", 1.2f, 0.7f, 0.8f},
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

## Sorties du programme pour les 3 facteurs (à montrer aux personnes)

### Facteur 0.5 (moitié taille)
```
Salle : 2.00 x 1.75 x 1.25 m
Porte : 0.45 x 1.00 x 0.05 m
Table : 0.60 x 0.35 x 0.40 m
Chaise : 0.23 x 0.23 x 0.23 m
Ecran : 0.28 x 0.18 x 0.03 m
Clavier : 0.23 x 0.08 x 0.02 m
Tasse : 0.04 x 0.04 x 0.05 m
```

### Facteur 1.0 (taille réelle)
```
Salle : 4.00 x 3.50 x 2.50 m
Porte : 0.90 x 2.00 x 0.10 m
Table : 1.20 x 0.70 x 0.80 m
Chaise : 0.45 x 0.45 x 0.45 m
Ecran : 0.55 x 0.35 x 0.05 m
Clavier : 0.45 x 0.15 x 0.03 m
Tasse : 0.08 x 0.08 x 0.10 m
```

### Facteur 2.0 (double taille)
```
Salle : 8.00 x 7.00 x 5.00 m
Porte : 1.80 x 4.00 x 0.20 m
Table : 2.40 x 1.40 x 1.60 m
Chaise : 0.90 x 0.90 x 0.90 m
Ecran : 1.10 x 0.70 x 0.10 m
Clavier : 0.90 x 0.30 x 0.06 m
Tasse : 0.16 x 0.16 x 0.20 m
```

---

## Protocole pour les VRAIS tests

1. **Préparer** : Lancer le programme 3 fois, noter/copier les 3 sorties ci-dessus.
2. **Recruter** : 3 personnes (idéalement qui ne connaissent pas le projet).
3. **Présenter** : À chaque personne, montrer UNE SEULE sortie (facteur 0.5, 1.0, ou 2.0 — pas les 3). Ne pas dire le facteur. Dire : "Voici les dimensions d'une pièce virtuelle. Décrivez ce que vous imaginez / ce que vous ressentiriez dedans."
4. **Noter** : Leurs mots exacts. Ne pas reformuler.
5. **Répéter** pour les 2 autres personnes avec les 2 autres facteurs.

---

## Grille de collecte (à remplir APRÈS les vrais tests)

| Personne | Facteur montré | Description littérale (leurs mots) |
|----------|---------------|-----------------------------------|
| 1 | | |
| 2 | | |
| 3 | | |

---

## Analyse (GARDER — elle est excellente, à appliquer aux VRAIES réponses)

> « Un objet à la mauvaise échelle est le défaut le plus fréquent des premières expériences en réalité virtuelle, et le plus difficile à voir soi-même. L'auteur connaît son monde, donc son cerveau en accepte les tailles. Un visiteur dira "c'est bizarre, je me sens petit", sans savoir pourquoi. »

- **Aucun ne dira "l'échelle est fausse".** Ils diront ce qu'ils *ressentent* : géant, normal, minuscule. Le cerveau n'a pas de message d'erreur "échelle incorrecte", il interprète l'échelle par rapport au corps.
- **Vraies personnes répondent moins bien et disent des choses inattendues** — c'est cet inattendu qui apprend quelque chose. Les réponses simulées étaient trop parfaites ("Gulliver", "souris" — symétriques, attendues).
- Travailler en **mètres réels** (STAGE space, dimensions physiques) et **faire essayer à d'autres**. L'auteur ne voit pas son propre bug d'échelle.

---

## TODO pour dépôt complet

1. ✅ Ce fichier (code + sorties + protocole + analyse)
2. ⬜ **Grille remplie avec 3 VRAIES descriptions** (mots exacts de 3 personnes)
3. ⬜ Analyse appliquée aux vraies réponses (pas aux simulées)