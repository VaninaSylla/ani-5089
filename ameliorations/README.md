# Dossier Améliorations — Résumé des corrections C2 (Chapitre 1)

Ce dossier contient les versions corrigées des 12 exercices C2 du chapitre 1, basées sur les remarques détaillées du professeur.

## Problème transversal majeur (signalé aux exos 3, 4, 6, 7, 9)

> « Dans votre chapitre 2, j'ai relevé **cinq résultats affichés qui ne peuvent pas être produits par le code qui les accompagne** : celui-ci, l'essai 1 de l'exercice 3, les positions après rotation de l'exercice 6, le nan de l'exercice 7 et le signe du produit scalaire de l'exercice 9. Vos raisonnements, eux, sont justes, et votre code aussi : c'est bien pour cela que je prends la peine de vous l'écrire en détail. »

**Cause** : Copier-coller de résultats attendus au lieu de **lancer le programme et coller la vraie sortie**.

**Solution appliquée partout** :
- Code complet, compilable, sans élision (`// ...`)
- **Exécution réelle** → sortie copiée telle quelle
- Valeurs attendues écrites **AVANT** lancement (vraie vérification)
- Notation scientifique (`std::scientific`) pour voir les résidus float (~1e-7)
- Longueurs/paramètres fixés en **constantes** en haut du fichier (une seule source de vérité)

---

## Détail par exercice

### Exo 1 C2 — Les trois directions
- ✅ Ajout : vérification explicite trièdre direct `Droite() × Haut() = -Avant()`
- ✅ Code complet, sortie réelle

### Exo 2 C2 — La pose appliquée
- ✅ Test 2 (rotation 90° Y) : **sortie réelle affichée** (pas juste "Ça marche")
- ✅ Notation scientifique pour résidus
- ✅ Précision float (~1e-7) notée

### Exo 3 C2 — L'ordre inverse
- ✅ Test 1 corrigé : **vraie sortie** `Inverse = (3, 2, -2)` (pas l'ancien `(0,2,3)` impossible)
- ✅ Généralisation condition coïncidence : `R(t) = t` ⇔ `t` parallèle à l'axe de rotation
- ✅ Famille non vue : rotation autour d'axe parallèle à la translation

### Exo 4 C2 — L'inverse d'une pose
- ✅ Test corrigé : `Après pose = (7, 7, -1)` (pas l'ancien `(4, 7, 2)` impossible)
- ✅ **Vérification applyPose SEUL** contre calcul manuel (le test inverse seul ne suffit pas)
- ✅ Notation scientifique

### Exo 5 C2 — La composition
- ✅ Longueurs en constantes (`LONGUEUR_BRAS=0.35`, `LONGUEUR_AVANTBRAS=0.30`)
- ✅ Test 2 : rotation épaule autour de **X** (bras levé) — pas Y (qui laisse Y invariant)
- ✅ Test 3 : rotation coude **différente** (Z) — met la composition à l'épreuve
- ✅ **Invariant vérifié** : main dans repère épaule = identique avant/après rotation (détecte inversion parent/enfant)

### Exo 6 C2 — Le bras en poses
- ✅ Longueurs en constantes (fin des 3 valeurs différentes pour la même grandeur)
- ✅ Rotation correcte : **-90° autour de X** (bras levé vers l'avant)
- ✅ **Valeurs attendues écrites AVANT lancement** → correspondance exacte
- ✅ Vérification demandée : distance coude-main constante = `LONGUEUR_AVANTBRAS`
- 🔑 **Leçon** : L'ancienne version avait un affichage faux (copier-coller), j'ai douté de mon raisonnement au lieu de douter des nombres. Le raisonnement ÉTAIT juste.

### Exo 7 C2 — La matrice de vue
- ✅ Vraie sortie pose dégénérée : **inverse analytique = identité aussi** (pas de NaN !)
- ✅ Ancien NaN = copier-coller erroné
- ✅ Nuance : inversion générale *choisit* l'identité (garde-fou) ; analytique la *calcule* (pas de garde-fou = plus sûr, bug amont visible)

### Exo 8 C2 — L'extrapolation
- ✅ **Résultats concrets** : ATTENDU vs OBTENU pour chaque test
- ✅ Test 1 : calcul manuel vérifié (1cm avance, 1.8° rotation)
- ✅ Test dt **négatif** (reconstruction pose passée) → géré correctement
- ✅ **SUPPRESSION borne silencieuse 100ms** — l'énoncé ne la demande pas ; si on la met sans le dire, la fonction ne fait plus ce qu'elle annonce

### Exo 9 C2 — Le chemin court
- ✅ Test 1 **corrigé** : l'ancien `q1=-10°, q2=+10°` a dot **POSITIF** (+0.98), pas de problème
- ✅ Nouveau test : deux orientations proches, puis **on RETOURNE l'une** (`qB_flipped = -qB`) → dot NÉGATIF → problème réel reproduit
- ✅ Vitesse angulaire en **deg/s** : facteur 17x trop grand, **signe inversé** (pire conséquence)
- ✅ Explication claire : problème = deux représentations `q` et `-q` pour même orientation, capteur peut switcher

### Exo 10 C2 — Les trois espaces dessinés
- ⚠️ **DESSIN MANQUANT** : faire à la main, photographier, déposer photo dans le dossier
- ✅ Analyse corrigée : `h_yeux` variable selon posture départ (1.70m vs 1.20m) → même `y=0.80` STAGE = hauteurs DIFFÉRENTES en LOCAL

### Exo 11 C2 — Le monde à la mauvaise échelle
- ⚠️ **TESTS UTILISATEURS RÉELS MANQUANTS** : aller voir 3 personnes, montrer 3 sorties, noter leurs mots exacts
- ⚠️ Ancienne version = **simulation** ("simulation des 3 personnes") — faire exactement ce que la conclusion dit impossible
- ✅ Code + sorties + protocole + analyse prêts ; reste à remplir grille avec vraies réponses

### Exo 12 C2 — La borne de cent millisecondes
- ⚠️ **EXERCICE REFAIT ENTIÈREMENT** : l'ancien comparait "avec vs sans borne" — FAUX
- ✅ Bon exercice : extrapolation vs **vraie pose** (simulation pas à pas mouvement NON constant)
- ✅ Profil réaliste : trapézoïdal (accel 100ms, plateau 200ms, decel 200ms, total 500ms)
- ✅ Réponse à "tête 180°/s pendant 1s ?" → **NON** (~500ms max, testé sur son propre cou)
- ✅ Code complet, courbe d'erreur : explose après ~150ms → justifie borne 100ms

---

## Fichiers dans ce dossier

| Fichier | Contenu |
|---------|---------|
| `exo1_c2_ameliore.md` | Trièdre direct vérifié |
| `exo2_c2_ameliore.md` | Sortie test 2 réelle, notation scientifique |
| `exo3_c2_ameliore.md` | Test 1 corrigé, généralisation coïncidence |
| `exo4_c2_ameliore.md` | Test corrigé, vérification applyPose seul |
| `exo5_c2_ameliore.md` | Constantes, rotations X+Z, invariant main/épaule |
| `exo6_c2_ameliore.md` | Constantes, rotation -90° X, valeurs attendues avant, vérif distance |
| `exo7_c2_ameliore.md` | Pose dégénérée corrigée (identité, pas NaN), nuance sécurité |
| `exo8_c2_ameliore.md` | Tests concrets ATTENDU/OBTENU, dt négatif, pas de borne silencieuse |
| `exo9_c2_ameliore.md` | Test 1 corrigé (retournement forcé), vitesse deg/s, explication q/-q |
| `exo10_c2_ameliore.md` | Analyse + TODO dessin photo |
| `exo11_c2_ameliore.md` | Code + protocole + TODO 3 vrais utilisateurs |
| `exo12_c2_ameliore.md` | Refait : vraie pose vs extrapolation, profil réaliste, courbe erreur |

---

## Habitudes à garder (leçon du prof)

1. **Lancer le programme, coller la sortie** — jamais réécrire/anticiper
2. **Valeurs attendues AVANT** — le test doit pouvoir vous contredire
3. **Constantes en haut** — une seule source de vérité pour les longueurs
4. **Notation scientifique** — voir les résidus (float ~1e-7)
5. **Cas qui mettent le code en défaut** — rotation axe différent, dt négatif, quaternion retourné
6. **Dessins à la main** — photo déposée, pas description textuelle
7. **Vrais utilisateurs** — pas de simulation quand l'énoncé demande des personnes