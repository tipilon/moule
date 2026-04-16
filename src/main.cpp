// ============================================================
//  main.cpp — moulé
//  ESP32 IoT — Point d'entrée principal
// ============================================================

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "config.h"
#include "contact_log.h"
#include "palette_monitor.h"
#include "wifi_manager.h"

// ── Instances globales ───────────────────────────────────────
static WiFiManager wifiManager;
static PaletteMonitor paletteMonitor;
static ContactLog contactLog;

// ── Prototypes ───────────────────────────────────────────────
static void setupOTA();
static void printBanner();

// ── setup() ─────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(200);
    printBanner();

    // WiFi
    wifiManager.setStatusLed(WIFI_STATUS_LED_PIN);
    wifiManager.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_HOSTNAME);
    if (!wifiManager.connect(WIFI_CONNECT_TIMEOUT_MS)) {
        Serial.println("[FATAL] Impossible de se connecter au WiFi. Redémarrage...");
        delay(3000);
        ESP.restart();
    }

#if OTA_ENABLED
    setupOTA();
#endif

    paletteMonitor.begin(PALETTE_PIN, PALETTE_LIGHT_PIN, PALETTE_TIMEOUT_MS);

    contactLog.begin();
    paletteMonitor.setOnContact([](bool isOn) { contactLog.addEvent(isOn); });

    Serial.println("[setup] Initialisation terminée — entrée dans loop()");
}

// ── loop() ──────────────────────────────────────────────────
void loop() {
    // Maintien de la connexion WiFi
    wifiManager.maintain();

#if OTA_ENABLED
    ArduinoOTA.handle();
#endif

    // Surveillance du signal palette
    paletteMonitor.update();

    // Traitement des requêtes web
    contactLog.handle();

    delay(10);
}

// ── Fonctions auxiliaires ────────────────────────────────────

static void setupOTA() {
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
        const String type = (ArduinoOTA.getCommand() == U_FLASH) ? "firmware" : "filesystem";
        Serial.printf("[OTA] Début mise à jour: %s\n", type.c_str());
    });
    ArduinoOTA.onEnd([]() { Serial.println("\n[OTA] Terminé — redémarrage"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] Progression: %u%%\r", (progress * 100) / total);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        static const char* errors[] = {"Auth Failed", "Begin Failed", "Connect Failed",
                                       "Receive Failed", "End Failed"};
        const uint8_t idx = (error >= 1 && error <= 5) ? error - 1 : 0;
        Serial.printf("[OTA] Erreur [%u]: %s\n", error, errors[idx]);
    });

    ArduinoOTA.begin();
    Serial.println("[OTA] Prêt");
}

static void printBanner() {
    Serial.println();
    Serial.println("╔══════════════════════════════════╗");
    Serial.println("║      moulé — ESP32 IoT v0.1.0    ║");
    Serial.println("╚══════════════════════════════════╝");
    Serial.printf("  Chip   : %s  Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("  Flash  : %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("  CPU    : %u MHz\n", ESP.getCpuFreqMHz());
    Serial.println("──────────────────────────────────");
}
