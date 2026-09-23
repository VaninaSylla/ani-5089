# Serie 2
## Exercice - DEUX YEUX, DEUX IMAGES
### 7 — Le coût du doublement
 

## Programme de test

J'ai repris le même programme et j'ai isolé le temps de rendu (le clear + present) sans la logique.

## Mesures

-Temps de rendu seul (1 œil) : 2,1 ms en moyenne, pic à 4,8 m
-Estimation pour 2 yeux : ~4,2 ms moyenne, ~9,6 ms pi

## Ce qu'il resterait pour le reste

À 90 Hz (budget total 11,1 ms, budget code 3,1 ms) :
- Rendu 2 yeux : ~4,2 ms (moyenne) →DÉJÀ PLUS que le budget code 
- Il resterait : 3,1 - 4,2 =-1,1 m (impossible)

À 72 Hz (budget code 5,9 ms) :
- Rendu 2 yeux : ~4,2 ms → Il reste1,7 m pour la logique, la physique, etc. C'est serré.

À 120 Hz (budget code 0,3 ms) :
- Complètement impossible.

## Conclusion

Le doublement du rendumange tout le budge. Il faut absolument réduire :
1. Le coût du rendu lui-même (moins de draw calls, shaders plus simples, moins de polygones)
2. Utiliser des optimisations comme le *single-pass stereo* ou *instanced rendering* pour ne pas tout refaire deux fois
3. Peut-être baisser la résolution par œil

C'est ce que le chapitre appelle « la colonne de droite est votre marge de manœuvre » — la logique, la physique, le chargement ne se doublent pas, mais le rendu oui. Si le rendu prend déjà plus que le budget code, y'a plus de marge du tout.