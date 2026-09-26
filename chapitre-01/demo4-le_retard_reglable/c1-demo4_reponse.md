# Démonstration 4 — Le retard réglable

## Déroulement

J'ai fait essayer mon programme (souris avec retard de 0 à 200 ms) à 3 volontaires devant la classe. Je montais le retard par paliers de 20 ms.

## Résultats au tableau

| Personne | Seuil « je le sens » | Commentaire |
|----------|---------------------|-------------|
| Volontaire 1 | 30 ms | « Là, c'est bizarre, la souris me suit pas » |
| Volontaire 2 | 50 ms | « Ah oui, là ça lag » |
| Volontaire 3 | 25 ms | « Dès 20 ms je voyais un décalage » |

**Moyenne classe : ~35 ms**

## Conclusion (ce qu'on a dit ensemble)

- Sur écran, le seuil est autour de **30-50 ms**.
- En VR, le budget est **20 ms mouvement→photon** (et ~10 ms pour notre code).
- **Le seuil VR est 2x plus bas** que le seuil écran.

## Pourquoi ? (synthèse de la classe + prof)

1. **Champ de vision** : Écran = 60° avec repères fixes autour. Casque = 100°+ sans repères fixes.
2. **Couplage tête** : Écran = main/souris. Casque = tête directement (vestibulaire).
3. **Conflit sensoriel** : Écran = lag visuel seulement. Casque = conflit vestibulaire/visuel.
4. **Proximité** : Écran à 60 cm. Casque à 3 cm (optique).

## Ce que ça implique pour le budget d'une image

> « Si les gens sentent 30 ms sur écran où y'a pas de conflit vestibulaire, en casque ils sentiront bien avant 20 ms. Donc le budget de 20 ms n'est pas une "marge de sécurité", c'est une **limite dure**. Et notre code n'a que ~10 ms dedans. »

Le prof : « C'est pour ça qu'on ne bloque jamais le thread qui suit la tête. Jamais. »