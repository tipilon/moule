#pragma once
// ============================================================
//  config_example.h — Template de configuration
//  USAGE : copier ce fichier en src/config.h
//          puis renseigner vos vraies valeurs.
//  NOTE  : src/config.h est dans .gitignore — ne jamais
//          committer de credentials dans le dépôt !
// ============================================================

// ── WiFi ────────────────────────────────────────────────────
#define WIFI_SSID "votre_ssid"
#define WIFI_PASSWORD "votre_mot_de_passe"
#define WIFI_HOSTNAME "moule-esp32"

// ── MQTT (optionnel) ─────────────────────────────────────────
#define MQTT_ENABLED false
#define MQTT_BROKER "192.168.1.100"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_CLIENT_ID WIFI_HOSTNAME
#define MQTT_TOPIC_BASE "moule/"

// ── OTA (Over-The-Air update) ────────────────────────────────
#define OTA_ENABLED true
#define OTA_HOSTNAME WIFI_HOSTNAME
#define OTA_PASSWORD "moule_ota_password"

// ── Timing ──────────────────────────────────────────────────
#define WIFI_CONNECT_TIMEOUT_MS 15000U

// ── LED statut WiFi ──────────────────────────────────────────
// GPIO 2 = LED intégrée sur la plupart des ESP32 DevKit
// Mettre à 255 pour désactiver
#define WIFI_STATUS_LED_PIN 2

// ── NTP (horloge réseau) ─────────────────────────────────────
// Serveur de temps (pool public, accessible sur Internet)
#define NTP_SERVER "pool.ntp.org"
// Fuseau horaire POSIX — exemples :
//   "EST5EDT,M3.2.0,M11.1.0"  → Est (Québec / Ontario)
//   "CST6CDT,M3.2.0,M11.1.0"  → Centre (Manitoba)
//   "MST7MDT,M3.2.0,M11.1.0"  → Montagne (Alberta)
//   "PST8PDT,M3.2.0,M11.1.0"  → Pacifique (C.-B.)
#define NTP_TIMEZONE "EST5EDT,M3.2.0,M11.1.0"

// ── Notifications ntfy.sh ────────────────────────────────────
// Topic unique — choisir un nom difficile à deviner
// Abonnement dans l'app : ntfy.sh/<NTFY_TOPIC>
#define NTFY_TOPIC "moule-alarme-CHANGER-CE-NOM"
#define NTFY_TITLE_ALARM "Alerte moule"
#define NTFY_TITLE_RESOLVED "Moule — retour normal"

// ── Palette Monitor ──────────────────────────────────────────
// GPIO branché à la sortie de l'opto-isolateur
#define PALETTE_PIN 4
// GPIO pour la lumière d'alarme (relais ou DEL)
#define PALETTE_LIGHT_PIN 25
// Délai max autorisé avant alarme en ms (ex : 30000 = 30 s)
#define PALETTE_TIMEOUT_MS 30000U
// Fenêtre de debounce anti-rebond en ms
#define PALETTE_DEBOUNCE_MS 50U
// true  = signal HIGH → palette active (pull-down interne)
// false = signal LOW  → palette active (pull-up  interne) ← opto NPN typique
#define PALETTE_ACTIVE_HIGH false
// true  = mettre la sortie HIGH pour allumer la lumière
// false = mettre la sortie LOW  pour allumer (relais actif-bas)
#define PALETTE_LIGHT_ACTIVE_HIGH true
