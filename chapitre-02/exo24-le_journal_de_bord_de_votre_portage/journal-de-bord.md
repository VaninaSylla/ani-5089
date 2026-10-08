# Journal de bord du portage

## Entrée 1
Symptôme :
$ jenga build
jenga : No .jenga workspace file found.

Commande lancée : `jenga build` (sans bloc `workspace` dans projet.jenga)
Message complet :
```
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║           ██╗███████╗███╗   ██╗ ██████╗  █████╗             ║
║           ██║██╔════╝████╗  ██║██╔════╝ ██╔══██╗            ║
║           ██║█████╗  ██╔██╗ ██║██║  ███╗███████║            ║
║           ██║██╔══╝  ██║╚██╗██║██║   ██║██╔══██║            ║
║           ╚█████╔╝███████╗██║ ╚████║╚██████╔╝██║  ██║║
║            ╚════╝ ╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝  ╚═╝            ║
║                                                                  ║
║             Multi-platform C/C++ Build System v2.8.4             ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝

jenga : No .jenga workspace file found.
```
J'ai cru : Mon fichier projet.jenga est mal nommé
C'était : Il manquait le bloc `workspace` englobant le `project`
Temps perdu : 30

## Entrée 2
Symptôme :
$ jenga build
Link failed: undefined symbol: ShowWindow

Commande lancée : `jenga build` (sans `links(["user32", ...])` dans le filtre Windows)
Message complet :
```
lld-link: error: undefined symbol: __declspec(dllimport) ShowWindow
>>> referenced by
C:\...\src\main.cpp:5
>>> 
C:\...\Build\Obj\Debug-Windows\MaSalle\src_main.obj:(main)
clang++: error: linker command failed with exit code 1 (use -v to see invocation)

┌──────────────────────────────────────────────────────────────────────┐
│  ✗ Build Failed                                                      │
│  Errors: 2  | Failed files: 1                                        │
└──────────────────────────────────────────────────────────────────────┘
```
J'ai cru : J'avais oublié d'inclure le header NKWindow
C'était : Il manquait `links(["user32", "gdi32", "opengl32", "dinput8", "dxguid", "winmm"])` dans le fichier .jenga
Temps perdu : 45

## Entrée 3
Symptôme :
$ adb devices -l
List of devices attached
R58M20       unauthorized usb:1-2 transport_id:2

Appareil utilisé : Samsung Galaxy S10 (R58M20), branché en USB mais non autorisé (boîte de dialogue dans le casque)
J'ai cru : Le câble USB est défectueux
C'était : Le casque n'était pas en mode "device" (boîte de dialogue d'autorisation dans le casque)
Temps perdu : 20

## Entrée 4
Symptôme :
$ adb logcat -d | grep -A 20 "Fatal signal"
SIGSEGV signal 11, fault addr 0x0 in libMaSalle.so (LireHauteurPlafond+12)

Commande lancée : Lancer l'appli sur le casque, puis `adb logcat -d`
Message complet :
```
--------- beginning of crash
09-29 00:19:42.786 29478 29502 F libc    : Fatal signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0x0 in tid 29502 (m.enspy.masalle), pid 29478 (m.enspy.masalle)
09-29 00:19:43.561 29515 29515 F DEBUG   : *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
09-29 00:19:43.561 29515 29515 F DEBUG   : Build fingerprint: 'samsung/e3qxeea/e3q:16/BP2A.250605.031.A3/S928BXXS5CZD1:user/release-keys'
09-29 00:19:43.561 29515 29515 F DEBUG   : Revision: '13'
09-29 00:19:43.561 29515 29515 F DEBUG   : ABI: 'arm64'
09-29 00:19:43.561 29515 29515 F DEBUG   : Processor: '4'
09-29 00:19:43.561 29515 29515 F DEBUG   : Timestamp: 2026-09-29 00:19:42.865575945+0100
09-29 00:19:43.561 29515 29515 F DEBUG   : Process uptime: 1s
09-29 00:19:43.561 29515 29515 F DEBUG   : Cmdline: cm.enspy.masalle
09-29 00:19:43.561 29515 29515 F DEBUG   : pid: 29478, tid: 29502, name: m.enspy.masalle  >>> cm.enspy.masalle <<<
09-29 00:19:43.561 29515 29515 F DEBUG   : uid: 10346
09-29 00:19:43.561 29515 29515 F DEBUG   : signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0x0000000000000000
09-29 00:19:43.561 29515 29515 F DEBUG   : Cause: null pointer dereference
09-29 00:19:43.561 29515 29515 F DEBUG   : backtrace:
09-29 00:19:43.561 29515 29515 F DEBUG   :       #00 pc 0000000000002fb0  /data/app/.../lib/arm64/libMaSalle.so (LireHauteurPlafond()+12)
09-29 00:19:43.561 29515 29515 F DEBUG   :       #01 pc 0000000000002fe8  /data/app/.../lib/arm64/libMaSalle.so (android_main+48)
09-29 00:19:43.561 29515 29515 F DEBUG   :       #02 pc 0000000000004314  /data/app/.../lib/arm64/libMaSalle.so (android_app_entry+264)
09-29 00:19:43.561 29515 29515 F DEBUG   :       #03 pc 0000000000082600  /apex/com.android.runtime/lib64/bionic/libc.so (__pthread_start(void*)+184)
09-29 00:19:43.561 29515 29515 F DEBUG   :       #04 pc 0000000000074a58  /apex/com.android.runtime/lib64/bionic/libc.so (__start_thread+68)
```
J'ai cru : Bug dans mon code C++
C'était : Architecture ABI manquante dans le paquet (pas de lib/arm64-v8a/)
Temps perdu : 60

## Entrée 5
Symptôme :
$ jenga sign --apk dummy.apk
jenga : Keystore not found. Provide --keystore or configure in project.

Commande lancée : `jenga sign --apk dummy.apk` (sans avoir généré de clé)
Message complet :
```
jenga : Keystore not found. Provide --keystore or configure in project.
```
J'ai cru : Problème de variables d'environnement
C'était : Clé de signature non générée (jenga sign --gen-key requis)
Temps perdu : 40

## Entrée 6
Symptôme :
Mesure Debug : 48 ms, Release : 9 ms → facteur 5, Debug dépasse budget 11 ms
J'ai cru : Mon algorithme est trop lent
C'était : Configuration Debug non optimisée (facteur 10x plus lent)
Temps perdu : 25

## Entrée 7
Symptôme :
Filtre `system:android` ignoré, pas de lien Windows → linker error si code Windows

Commande lancée : `jenga build` avec `filter("system:android")` au lieu de `filter("system:windows")`
Message complet : Le build réussit (filtre ignoré silencieusement). La machine rapporte `system=windows` pas `system=android`. Les bibliothèques Windows ne sont pas liées.
J'ai cru : Syntaxe du filtre incorrecte
C'était : La machine rapporte `system=linux` pas `system=android`
Temps perdu : 15