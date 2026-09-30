# Journal de bord du portage

## Entrée 1
Symptôme : jenga build échoue avec "No .jenga workspace file found"
J'ai cru : Mon fichier projet.jenga est mal nommé
C'était : Il manquait le bloc `workspace` englobant le `project`
Temps perdu : 30

## Entrée 2
Symptôme : Erreur de lien "undefined reference to NkWindowOpen"
J'ai cru : J'avais oublié d'inclure le header NKWindow
C'était : Il manquait `links(["NKWindow"])` dans le fichier .jenga
Temps perdu : 45

## Entrée 3
Symptôme : adb devices ne montre pas le casque branché
J'ai cru : Le câble USB est défectueux
C'était : Le casque n'était pas en mode "device" (boîte de dialogue d'autorisation dans le casque)
Temps perdu : 20

## Entrée 4
Symptôme : Programme installé mais crash au démarrage sur le casque
J'ai cru : Bug dans mon code C++
C'était : Architecture ABI manquante dans le paquet (pas de lib/arm64-v8a/)
Temps perdu : 60

## Entrée 5
Symptôme : jenga package échoue "keystore not found"
J'ai cru : Problème de variables d'environnement
C'était : Clé de signature non générée (jenga sign --gen-key requis)
Temps perdu : 40

## Entrée 6
Symptôme : Mesures de performance incohérentes entre Debug et Release
J'ai cru : Mon algorithme est trop lent
C'était : Configuration Debug non optimisée (facteur 10x plus lent)
Temps perdu : 25

## Entrée 7
Symptôme : Filtre system:android ne s'applique pas
J'ai cru : Syntaxe du filtre incorrecte
C'était : La machine rapporte `system=linux` pas `system=android`
Temps perdu : 15