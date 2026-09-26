# Démonstration 1 — Le budget au tableau

## Déroulement

J'ai dessiné une barre de 20 cm au tableau (1 cm = 1 ms). J'ai demandé à la classe de placer les 5 étapes à leur échelle.

## Ce qui a été placé (consensus de la classe)

| Étape | Position sur la barre (ms) | Commentaire classe |
|-------|---------------------------|-------------------|
| Capteurs mesurent | 1-2 ms (début) | « C'est rapide » |
| Transmission système | 1-3 ms | « Ça dépend du bus » |
| **Notre code (rendu)** | **5-11 ms** | **« C'EST NOUS LE GOULOT »** |
| Compositeur | 1-2 ms | « Invisible » |
| Affichage écran | 2-5 ms | « Persistance » |

## Ce qui reste pour le code

La barre fait 20 cm. Les 4 autres étapes prennent ~5-12 ms au total. Il reste **8 à 15 ms** pour notre code selon les optimismes.

Mais le prof a fait remarquer : « C'est le *pire cas* qui compte. Si votre code prend 11 ms une fois sur 100, l'image est en retard. »

## Réaction de la classe

- « Putain, c'est rien 11 ms » 
- « Moi mon jeu tourne à 16 ms par frame en moyenne... »
- « Donc si je fais un truc un peu lourd, ça marche pas en VR ? »
- Le prof : « En VR, tu n'as pas droit à la moyenne. Tu as droit au pic. »

## Ce que j'ai retenu

Le budget n'est pas « 20 ms pour nous ». C'est « 20 ms total, dont 8-12 ms sont déjà pris, et il faut tenir le coup *toujours* ». La marge est mince, et elle est pour le *pire cas*, pas la moyenne.