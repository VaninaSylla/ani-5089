# Serie 2
## Exercices - DEUX YEUX, DEUX IMAGES
### 2 — Le tableau des budgets


| Étape | Valeur trouvée | Source |
|---|---:|---|
| Les capteurs mesurent le mouvement | Valeur précise non trouvée |Les fabricants de casques VR (Oculus/Meta, HTC Vive, Valve Index) publient des latences globales « motion-to-photon » (souvent 20–30 ms), mais ils ne détaillent pas séparément le temps de mesure du capteur. Exemple : Oculus Latency Tester et articles IEEE sur la latence VR indiquent que la mesure des capteurs est incluse dans la chaîne complète.|
| Le système transmet la mesure | Valeur précise non trouvée | La transmission dépend du protocole interne (USB, Bluetooth, Wi-Fi propriétaire). Les fiches techniques ne donnent pas de chiffre isolé. Par exemple, le SteamVR Tracking ou l’API Oculus Link décrivent la chaîne de communication, mais sans publier un temps de transmission spécifique.|
| L'application décide et dessine | 11,1 ms à 90 Hz | OpenXR synchronise l'application avec l'affichage grâce à `xrWaitFrame`. À 90 Hz, une image dure \(1000 / 90 = 11,1\) ms. [OpenXR](https://registry.khronos.org/OpenXR/specs/1.1/man/html/xrWaitFrame.html) |
| Le compositeur assemble l'image | Quelques millisecondes avant l'affichage | Le compositeur ajuste la position de la tête juste avant l’affichage [Meta](https://developers.meta.com/horizon/documentation/unreal/os-compositor/) |
| L'écran affiche l'image | 13,89 ms à 72 Hz, 11,11 ms à 90 Hz et 8,33 ms à 120 Hz | Ces valeurs correspondent à la durée d'une image selon la fréquence d'affichage Relation directe entre fréquence et durée d’une image : 1000/hz. Confirmé par la VR & AR Wiki [VR & AR Wiki](https://vrarwiki.com/wiki/Refresh_rate?utm_source=copilot.com)|

## Remarque

Les deux premières valeurs sont difficiles à trouver séparément. Elles dépendent
du casque et du système utilisé.

