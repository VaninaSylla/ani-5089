# Exercice 10 — Les trois espaces, dessinés

## Dessin (description textuelle)

Puisque je ne peux pas dessiner dans un fichier markdown, voici la description précise du dessin à faire sur papier :

### Vue de côté de la pièce (mur ouest à gauche, mur est à droite)

```
        PLAFOND (y = 2,50 m)
        ┌─────────────────────────────────────┐
        │                                     │
        │     ● ORIGINE STAGE (0,0,0)         │  ← Au SOL, centre zone de jeu
        │     │                               │
        │     │                               │
        │     │    ┌─────────┐                │
        │     │    │  TABLE  │  y = 0,80 m   │  ← Dans STAGE : table à 80 cm du sol
        │     │    │  0,80m  │                │
        │     │    └─────────┘                │
        │     │                               │
        │     ▼                               │
        │  SOL (y = 0)                        │
        │                                     │
        │           UTILISATEUR DEBOUT        │
        │           ● TÊTE (y ~ 1,70 m)       │
        │           │                         │
        │           ● ORIGINE VIEW (entre yeux)│ ← Suit la tête
        │           │                         │
        │           ● ORIGINE LOCAL (pieds)   │ ← Pose tête au démarrage, alignée gravité
        │                                     │
        └─────────────────────────────────────┘
        MUR OUEST                    MUR EST
```

### Placement de la table (80 cm haut) dans les 3 espaces

| Espace | Origine | Position table (centre) | Où elle "se retrouve" visuellement |
|--------|---------|------------------------|-----------------------------------|
| **STAGE** | Sol, centre zone | `(0, 0.80, 0)` | **Correcte** : sur le sol, 80 cm haut. `y=0` = plancher. |
| **LOCAL** | Tête au démarrage (alignée gravité) | `(0, 0.80 - 1.70, 0)` = `(0, -0.90, 0)` | **90 cm SOUS les yeux** de l'utilisateur ! Si l'utilisateur a démarré debout (yeux à 1,70m), la table est sous son nez. S'il a démarré assis (yeux à 1,20m), elle est à 40 cm sous les yeux. **Inutilisable pour un décor.** |
| **VIEW** | Entre les yeux (suit la tête) | `(0, 0.80 - 1.70, distance)` ≈ `(0, -0.90, 1.5)` | **Devant les yeux, en l'air** ! La table suit le regard. Pas ancrée au sol. |

### Ce que ça montre (le point du chapitre)

> « STAGE est le seul espace où y = 0 veut dire le plancher, donc le seul où poser un décor a un sens physique. En LOCAL, une table à quatre-vingts centimètres serait à quatre-vingts centimètres sous les yeux de quelqu'un qui était peut-être debout, peut-être assis. »

- **STAGE** : Origine fixe au sol. `y` = hauteur réelle. Idéal pour la salle, les meubles, le décor.
- **LOCAL** : Origine = pose tête au démarrage. `y=0` = hauteur des yeux au démarrage. Un objet à `y=0.8` est à 80 cm **au-dessus des yeux de départ**. Problème : l'utilisateur peut être debout OU assis au démarrage → même coordonnée = hauteur physique différente.
- **VIEW** : Origine = entre les yeux, suit la tête. `y=0` = toujours au niveau des yeux. Objet à `y=0.8` = toujours 80 cm au-dessus des yeux. Pour HUD, réticules, pas pour décor.

## Conclusion

Pour "bâtir ma salle" (chapitre 7+), **il faut utiliser STAGE**. C'est le seul où `(x, 0.80, z)` veut dire "sur le sol, 80 cm de haut" peu importe où l'utilisateur est, comment il a démarré, ou où il regarde.