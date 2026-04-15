# moulé — ESP32 IoT

[![CI](https://github.com/VOTRE_USERNAME/moule/actions/workflows/ci.yml/badge.svg)](https://github.com/VOTRE_USERNAME/moule/actions/workflows/ci.yml)
[![Release](https://github.com/VOTRE_USERNAME/moule/actions/workflows/release.yml/badge.svg)](https://github.com/VOTRE_USERNAME/moule/releases)

Projet ESP32 IoT développé avec PlatformIO + Arduino framework.

## Prérequis

- [VSCode](https://code.visualstudio.com/) + extension [PlatformIO IDE](https://platformio.org/install/ide?install=vscode)
- ESP32 (DevKitC ou compatible)
- Câble USB

## Démarrage rapide

```bash
# 1. Cloner le dépôt
git clone https://github.com/VOTRE_USERNAME/moule.git
cd moule

# 2. Créer votre fichier de configuration (NE PAS committer)
cp src/config_example.h src/config.h
# Éditer src/config.h avec vos identifiants WiFi

# 3. Compiler et flasher
pio run --target upload --environment esp32dev

# 4. Moniteur série
pio device monitor
```

## Structure du projet

```
moule/
├── .github/
│   ├── workflows/
│   │   ├── ci.yml          # Build + lint sur chaque push/PR
│   │   └── release.yml     # Publication automatique sur tag vX.Y.Z
│   ├── ISSUE_TEMPLATE/     # Templates de bugs et features
│   └── pull_request_template.md
├── src/
│   ├── main.cpp            # Point d'entrée
│   ├── wifi_manager.h/.cpp # Gestion WiFi avec reconnexion auto
│   └── config_example.h   # Template de configuration (copier → config.h)
├── .clang-format           # Style de code
├── .gitignore
└── platformio.ini          # Configuration PlatformIO
```

## Configuration

Copier `src/config_example.h` en `src/config.h` et renseigner :

| Paramètre | Description |
|-----------|-------------|
| `WIFI_SSID` | SSID de votre réseau WiFi |
| `WIFI_PASSWORD` | Mot de passe WiFi |
| `WIFI_HOSTNAME` | Nom mDNS de l'ESP32 |
| `OTA_PASSWORD` | Mot de passe pour les mises à jour OTA |

> **Important :** `src/config.h` est dans `.gitignore` — ne jamais le committer.

## Workflow Git

```
main        ← production stable, releases taggées
  └─ develop ← intégration, base des PRs
       └─ feature/xxx ← développement de fonctionnalités
       └─ fix/xxx     ← corrections de bugs
```

## Créer une release

```bash
git tag v1.0.0
git push origin v1.0.0
```
Le workflow GitHub Actions compile le firmware et crée automatiquement une release avec le binaire `.bin`.

## OTA (mise à jour sans fil)

Avec l'ESP32 connecté et en fonctionnement :

```bash
pio run --target upload --environment esp32dev \
        --upload-port moule-esp32.local
```

## Licence

MIT — voir [LICENSE](LICENSE)
