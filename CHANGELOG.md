# Changelog

Toutes les modifications notables sont documentées ici.
Format : [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/)
Versionnement : [Semantic Versioning](https://semver.org/lang/fr/)

---

## [Non publié]

---

## [0.2.0] — 2026-04-19

### Ajouté
- `PaletteMonitor` : surveillance du signal palette via opto-isolateur,
  alarme lumineuse, debounce, timeout configurable, callbacks ON/OFF et alarme
- `ContactLog` : journal des événements horodatés (NTP), serveur HTTP port 80
  consultable par navigateur, tampon circulaire de 100 entrées
- `Notifier` : notifications push via ntfy.sh sur alarme et retour à la normale
- `WiFiManager` : LED de statut (clignotement lent = connecté, rapide = déco)
- `.clangd` : limite l'indexation à `src/` pour améliorer les performances VSCode
- `config_example.h` : ajout des defines NTP, ntfy.sh et Palette Monitor

### Modifié
- CI : clang-format passe en mode auto-fix (commit automatique) au lieu de bloquer

---

## [0.1.0] — 2026-04-19

### Ajouté
- Structure initiale du projet PlatformIO + Arduino framework
- `WiFiManager` avec reconnexion automatique
- Support OTA (Over-The-Air update)
- CI GitHub Actions (build multi-environnements + lint)
- Release automatique sur tag `vX.Y.Z`
- Templates PR et issues GitHub
