# Serie 2
## Exercices - DEUX YEUX, DEUX IMAGES
### 8 — Le champ de vision asymétrique



## Recherche : Meta Quest 2 (exemple de casque du commerce)

J’ai choisi l’Oculus Rift DK1, un casque de réalité virtuelle commercial documenté dans le Oculus Rift Developer Guide.

Pour l’œil gauche, les quatre demi-angles du champ de vision sont :
| Direction du regard              | Angle |
| -------------------------------- | ----- |
| Vers le haut                     | 53,6° |
| Vers le bas                      | 58,9° |
| Vers le nez — côté nasal         | 50,3° |
| Vers l’extérieur — côté temporal | 58,7° |

Ainsi, pour l’œil gauche :

FOV horizontal= 50,3∘+58,7∘= 109,0∘
FOV vertical= 53,6∘+58,9∘= 112,5∘

Les quatre angles sont donc : (50,3∘ ,58,7∘, 53,6∘, 58,9∘) dans l'ordre (nez, exterieur,haut,bas)

## Interprétation selon le cours
Le cours explique que le champ de vision d’un œil dans un casque est asymétrique, car la lentille n’est pas centrée exactement sur l’œil et parce que la géométrie du visage limite davantage la vision vers le nez. Le cours indique donc qu’il faut utiliser quatre angles différents au lieu d’un seul angle de vision symétrique.

Cette représentation est également utilisée dans les interfaces de réalité virtuelle : les quatre valeurs correspondent aux angles gauche, droit, haut et bas du champ de vision de chaque œil. OpenXR prévoit aussi des champs de vision distincts pour les vues de l’œil gauche et de l’œil droit.

Les mesures du Rift DK1 confirment cette asymétrie : l’œil gauche voit davantage vers l’extérieur, avec 58,7∘ que vers le nez, avec 50,3∘. Il voit également davantage vers le bas, avec 58,9∘ que vers le haut, avec 53,6∘

## Source :
- Oculus Rift Developer Guide, exemple du champ de vision de l’œil gauche du DK1 53,6∘ vers le haut, 58,9∘ vers le bas 50,3∘ vers le nez et 58,7∘ vers l’extérieur. [centers.ulbsibiu](https://centers.ulbsibiu.ro/incon/wp-content/uploads/oculus.pdf)
- OpenXR / documentation technique sur les quatre angles FOV par œil : gauche, droite, haut et bas. [arxiv](https://arxiv.org/html/2506.02380)
- Étude sur les casques VR grand public : le champ de vision dépend du casque, de la distance œil-lentille et de l’écart interpupillaire. [link.springer](https://link.springer.com/article/10.1007/s10055-021-00619-x?error=cookies_not_supported&code=322b8a09-21bd-48ba-be96-422e37ed302b)

## Si on utilisait un champ symétrique de même surface
Si l’on employait à la place un champ de vision symétrique de même surface, la surface visible resterait approximativement la même, mais elle serait mal positionnée : l’image serait déformée ou décalée sur les bords, avec une correspondance incorrecte entre l’œil et la scène virtuelle




