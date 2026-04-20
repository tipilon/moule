// ============================================================
//  notifier.cpp — moulé
//  Voir notifier.h pour la description complète.
// ============================================================

#include "notifier.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "config.h"

// ── begin() ──────────────────────────────────────────────────
void Notifier::begin() {
    Serial.printf("[Notifier] ntfy.sh — topic : %s\n", NTFY_TOPIC);
}

// ── sendAlert() ───────────────────────────────────────────────
bool Notifier::sendAlert(const String& message) {
    return _post(message, NTFY_TITLE_ALARM, "urgent", "warning,rotating_light");
}

// ── sendResolved() ────────────────────────────────────────────
bool Notifier::sendResolved(const String& message) {
    return _post(message, NTFY_TITLE_RESOLVED, "default", "white_check_mark");
}

// ── _post() ───────────────────────────────────────────────────
bool Notifier::_post(const String& message, const char* title, const char* priority,
                     const char* tags) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Notifier] WiFi absent — notification annulée");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();  // pas de vérification du certificat (usage IoT)

    HTTPClient http;
    const String url = String("https://ntfy.sh/") + NTFY_TOPIC;

    if (!http.begin(client, url)) {
        Serial.println("[Notifier] échec HTTP begin");
        return false;
    }

    http.addHeader("Content-Type", "text/plain; charset=utf-8");
    http.addHeader("Title", title);
    http.addHeader("Priority", priority);
    http.addHeader("Tags", tags);

    const int code = http.POST(message);
    http.end();

    if (code == 200) {
        Serial.printf("[Notifier] OK → %s\n", title);
        return true;
    }

    Serial.printf("[Notifier] erreur HTTP %d\n", code);
    return false;
}
