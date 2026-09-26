# Exercice 10 — Les trois espaces, dessinés (Amélioré)

## ⚠️ DESSIN MANQUANT — À FAIRE

**L'énoncé demande : « Sans écrire de code, dessinez sur une même feuille » et « Rendez le dessin ».**

Plusieurs camarades l'ont fait : **dessinez au crayon, photographiez, déposez la photo dans le dossier de l'exercice à côté de ce fichier.** C'est accepté et c'est ce qui était attendu.

Le dessin montre ce que le tableau ne peut pas montrer : sur une seule feuille, avec les trois origines placées et la même table reportée trois fois, on voit d'un coup d'œil que les trois tables ne sont pas au même endroit, et de combien. Un défaut d'espace se repère à l'œil et jamais dans une liste de coordonnées — c'est tout le propos du chapitre.

---

## Analyse des trois espaces (Tableau corrigé)

| Espace | Origine | Position table (centre) | Où elle "se retrouve" visuellement |
|--------|---------|------------------------|-----------------------------------|
| **STAGE** | Sol, centre zone de jeu | `(0, 0.80, 0)` | **Correcte** : sur le sol, 80 cm haut. `y=0` = plancher. |
| **LOCAL** | Tête au démarrage (alignée gravité) | `(0, 0.80 - h_yeux, 0)` | **Problème** : `h_yeux` = 1.70m (debout) → table à -0.90m (90 cm SOUS les yeux). `h_yeux` = 1.20m (assise) → table à -0.40m (40 cm sous les yeux). Même coordonnée = hauteur physique DIFFÉRENTE selon posture au démarrage. **Inutilisable pour un décor.** |
| **VIEW** | Entre les yeux (suit la tête) | `(0, 0.80 - h_yeux_actuel, distance_tete_table)` | **Devant les yeux, en l'air** ! La table suit le regard. Pas ancrée au sol. Pour HUD/reticules uniquement. |

### Mesures réelles (reprises de l'exo 10 c1)
- Pièce : 4.00m × 3.50m, plafond 2.50m
- Table : 1.20m × 0.70m, plateau à 0.80m du sol
- Tasse : 8cm diamètre, sur la table
- Origine STAGE : coin sud-ouest au sol (0,0,0)
- Table centrée : centre à (2.00, 0.80, 1.75) dans STAGE

### Dans LOCAL (utilisateur debout au démarrage, yeux à 1.70m, tête à l'origine STAGE)
- Origine LOCAL = pose tête au démarrage = (2.00, 1.70, 1.75) dans STAGE
- Table dans LOCAL = (2.00, 0.80, 1.75) - (2.00, 1.70, 1.75) = **(0, -0.90, 0)**
- → 90 cm SOUS les yeux

### Dans LOCAL (utilisateur assis au démarrage, yeux à 1.20m)
- Origine LOCAL = (2.00, 1.20, 1.75)
- Table dans LOCAL = (2.00, 0.80, 1.75) - (2.00, 1.20, 1.75) = **(0, -0.40, 0)**
- → 40 cm SOUS les yeux

**Même coordonnée `y=0.80` dans STAGE = hauteurs DIFFÉRENTES dans LOCAL selon posture de départ.**

### Dans VIEW (utilisateur debout, tête à (2.00, 1.70, 1.75), regarde la table à 1.50m)
- Origine VIEW = entre les yeux = pose tête actuelle
- Table dans VIEW ≈ rotation pour regarder la table + translation
- → La table "flotte" devant les yeux, ne reste pas au sol quand on bouge la tête

---

## Conclusion (inchangée, correcte)

Pour "bâtir ma salle" (chapitre 7+), **il faut utiliser STAGE**. C'est le seul où `(x, 0.80, z)` veut dire "sur le sol, 80 cm de haut" peu importe où l'utilisateur est, comment il a démarré, ou où il regarde.

**VIEW** pour ce qui doit suivre le regard (HUD, réticules).
**STAGE** pour ce qui appartient au lieu (murs, meubles, décor).

---

## TODO pour dépôt complet

1. ✅ Ce fichier markdown (analyse)
2. ⬜ **Photo du dessin** (crayon sur papier, une feuille, trois origines + table reportée 3×) déposée dans `chapitre-01/exo10-les_trois_espaces_dessines/` à côté de ce fichier