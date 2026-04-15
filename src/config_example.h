#pragma once
// ============================================================
//  config_example.h — Template de configuration
//  USAGE : copier ce fichier en src/config.h
//          puis renseigner vos vraies valeurs.
//  NOTE  : src/config.h est dans .gitignore — ne jamais
//          committer de credentials dans le dépôt !
// ============================================================

// ── WiFi ────────────────────────────────────────────────────
#define WIFI_SSID              "votre_ssid"
#define WIFI_PASSWORD          "votre_mot_de_passe"
#define WIFI_HOSTNAME          "moule-esp32"

// ── MQTT (optionnel) ─────────────────────────────────────────
#define MQTT_ENABLED           false
#define MQTT_BROKER            "192.168.1.100"
#define MQTT_PORT              1883
#define MQTT_USER              ""
#define MQTT_PASSWORD          ""
#define MQTT_CLIENT_ID         WIFI_HOSTNAME
#define MQTT_TOPIC_BASE        "moule/"

// ── OTA (Over-The-Air update) ────────────────────────────────
#define OTA_ENABLED            true
#define OTA_HOSTNAME           WIFI_HOSTNAME
#define OTA_PASSWORD           "moule_ota_password"

// ── Timing ──────────────────────────────────────────────────
#define WIFI_CONNECT_TIMEOUT_MS   15000U
#define RECONNECT_DELAY_MS         5000U
