// ============================================================
//  wifi_manager.cpp
// ============================================================

#include "wifi_manager.h"

// ── Helpers internes ─────────────────────────────────────────

void WiFiManager::_log(const char* level, const char* msg) const {
    Serial.printf("[WiFiManager][%s] %s\n", level, msg);
}

// ── API publique ─────────────────────────────────────────────

void WiFiManager::begin(const char* ssid, const char* password,
                        const char* hostname) {
    _ssid     = ssid;
    _password = password;
    _hostname = hostname;

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);  // On gère la reconnexion nous-mêmes

    if (_hostname && strlen(_hostname) > 0) {
        WiFi.setHostname(_hostname);
    }

    _log("INFO", "Initialisé en mode STA");
}

bool WiFiManager::connect(uint32_t timeoutMs) {
    if (WiFi.status() == WL_CONNECTED) return true;

    _log("INFO", "Connexion en cours...");
    WiFi.begin(_ssid, _password);

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start >= timeoutMs) {
            _log("ERROR", "Timeout — connexion échouée");
            return false;
        }
        delay(250);
        Serial.print('.');
    }
    Serial.println();

    char buf[64];
    snprintf(buf, sizeof(buf), "Connecté — IP: %s  RSSI: %d dBm",
             WiFi.localIP().toString().c_str(), WiFi.RSSI());
    _log("INFO", buf);
    return true;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
    _log("INFO", "Déconnecté");
}

void WiFiManager::maintain() {
    if (WiFi.status() == WL_CONNECTED) return;

    const uint32_t now = millis();
    if (now - _lastReconnectAttempt < RECONNECT_DELAY_MS) return;

    _lastReconnectAttempt = now;
    _reconnectCount++;

    char buf[32];
    snprintf(buf, sizeof(buf), "Reconnexion #%lu...", (unsigned long)_reconnectCount);
    _log("WARN", buf);

    connect();
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getLocalIP() const {
    return WiFi.localIP().toString();
}

int WiFiManager::getRSSI() const {
    return WiFi.RSSI();
}
