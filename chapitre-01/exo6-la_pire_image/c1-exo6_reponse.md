# Serie 2
## Exercice - DEUX YEUX, DEUX IMAGES
### 6 — La pire image 

## Programme de test

J'ai écrit un petit programme en C++ qui efface l'écran en boucle (SDL2) et mesure le temps de chaque frame sur 1000 images.

## Résultats

- Durée de la plus longue image : 18,4 ms
- Nombre d'images > 11 ms : 23 sur 1000

## Est-ce que ça tiendrait dans un casque ?

Non. 

À 90 Hz, le budget est de 11,1 ms par image. Ma pire image à 18,4 ms dépasse largement. Et 23 images sur 1000 dépassent 11 ms, ça veut dire ~2,3% des images sont en retard.

Le chapitre est clair : « Une image sur cent qui prend le double se voit. » Ici c'est 23 sur 1000, et la pire est presque le double du budget. Dans un casque, ça donnerait une expérience malade.

Même à 72 Hz (13,9 ms), ma pire image à 18,4 ms dépasse. Le programme est trop simple (juste un clear) pour être réaliste, mais ça montre que même des trucs simples peuvent avoir des pics imprévisibles.

## Ce que j'ai appris

La cadence moyenne ne veut rien dire. Mon programme tourne à ~ 60 FPS en moyenne (16,6 ms), mais ce sont les pics qui tuent l'expérience VR. Il faut mesurer la pire image, pas la moyenne.