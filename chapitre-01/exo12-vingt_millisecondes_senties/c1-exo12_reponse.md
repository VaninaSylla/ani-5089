# Exercice 12 — Vingt millisecondes, senties

## Programme

J'ai fait un petit programme Python (pygame) qui affiche un cercle qui suit la souris avec un retard réglable (slider de 0 à 200 ms). 5 personnes l'ont testé.

## Seuils de détection (retard à partir lequel la personne dit « ah oui je le sens »)

| Personne | Seuil ressenti |
|----------|----------------|
| Moi      | ~35 ms         |
| Personne 1 | ~25 ms       |
| Personne 2 | ~45 ms       |
| Personne 3 | ~30 ms       |
| Personne 4 | ~50 ms       |


Moyenne : ~37 ms


## Comparaison au budget de 20 ms

Le seuil moyen sur écran (~37 ms) est 
presque le double
 du budget VR (20 ms mouvement→photon).

## Pourquoi le seuil est bien plus bas dans un casque

1. 
Champ de vision total : Sur écran, mes yeux voient aussi le bureau, le mur, la fenêtre qui ne bougent pas. Ça donne un repère stable. Dans un casque, tout
le champ de vision bouge avec le retard. Pas derepère externe.

2. 
Couplage tête-œil : Sur écran, je bouge la souris, pas ma tête. Dans le casque, je bouge ma tête . L'oreille interne dit « tête tournée de 10° », les yeux disent « image pas encore tournée ». Conflit direct. Sur écran, l'oreille interne dit « tête immobile », pas de conflit.

3. 
Proximité de l'écran: L'écran du casque est à ~3 cm de l'œil (via lentilles). Un petit décalage angulaire = grand décalage rétinien. Sur mon écran à 60 cm, le même retard angulaire est moins visible.

4. 
Pas de flou de mouvement naturel: Dans la vraie vie, quand je tourne la tête vite, il y a du flou de mouvement rétinien. Le casque doit le reproduire. Un retard ajoute un décalage en plus
du flou, ce qui n'existe pas dans la nature.


En résumé: Sur écran, le retard est un « lag » visuel. Dans le casque, c'est une contradiction sensorielle(vestibulaire vs visuel). Le cerveau tolère le lag, pas la contradiction.