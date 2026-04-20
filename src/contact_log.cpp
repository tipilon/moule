// ============================================================
//  contact_log.cpp — moulé
//  Voir contact_log.h pour la description complète.
// ============================================================

#include "contact_log.h"

#include <WiFi.h>

#include "config.h"

// ── begin() ──────────────────────────────────────────────────
void ContactLog::begin() {
    // Synchronisation NTP — l'heure est obtenue en arrière-plan
    configTzTime(NTP_TIMEZONE, NTP_SERVER);
    Serial.printf("[ContactLog] NTP configuré (%s / %s)\n", NTP_TIMEZONE, NTP_SERVER);

    // Enregistrement des routes HTTP
    _srv.on("/", [this]() { _handleRoot(); });
    _srv.on("/clear", [this]() { _handleClear(); });
    _srv.onNotFound([this]() { _srv.send(404, "text/plain", "Non trouve"); });

    _srv.begin();
    Serial.printf("[ContactLog] Serveur web: http://%s/\n", WiFi.localIP().toString().c_str());
}

// ── handle() ─────────────────────────────────────────────────
void ContactLog::handle() {
    _srv.handleClient();
}

// ── addEvent() ───────────────────────────────────────────────
void ContactLog::addEvent(bool isOn) {
    // Indice d'écriture = juste après la dernière entrée
    uint8_t idx = (_head + _count) % MAX_ENTRIES;

    if (_count == MAX_ENTRIES) {
        // Tampon plein — avancer la tête (écrase la plus ancienne)
        _head = (_head + 1) % MAX_ENTRIES;
    } else {
        _count++;
    }

    _buf[idx].ts = time(nullptr);
    _buf[idx].isOn = isOn;
    Serial.printf("[ContactLog] %s à %s\n", isOn ? "ON" : "OFF", _fmtTs(_buf[idx].ts).c_str());
}

// ── _handleRoot() ────────────────────────────────────────────
void ContactLog::_handleRoot() {
    _srv.send(200, "text/html; charset=utf-8", _buildHtml());
}

// ── _handleClear() ───────────────────────────────────────────
void ContactLog::_handleClear() {
    _count = 0;
    _head = 0;
    _srv.sendHeader("Location", "/");
    _srv.send(303, "text/plain", "");
}

// ── _fmtTs() ─────────────────────────────────────────────────
// Retourne la date/heure locale si NTP est synchro,
// sinon "T+Xs" (secondes depuis le démarrage de l'ESP32).
String ContactLog::_fmtTs(time_t t) const {
    if (t < 1000000000UL) {
        char buf[20];
        snprintf(buf, sizeof(buf), "T+%lus", (unsigned long) t);
        return String(buf);
    }
    struct tm* ti = localtime(&t);
    char buf[24];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ti);
    return String(buf);
}

// ── _fmtDuration() ───────────────────────────────────────────
String ContactLog::_fmtDuration(uint32_t sec) const {
    if (sec == 0) return F("&lt;1s");
    char buf[16];
    if (sec < 60) {
        snprintf(buf, sizeof(buf), "%lus", (unsigned long)sec);
    } else if (sec < 3600) {
        snprintf(buf, sizeof(buf), "%lum%02lus", (unsigned long)(sec / 60), (unsigned long)(sec % 60));
    } else {
        snprintf(buf, sizeof(buf), "%luh%02lum", (unsigned long)(sec / 3600), (unsigned long)((sec % 3600) / 60));
    }
    return String(buf);
}

// ── _buildHtml() ─────────────────────────────────────────────
String ContactLog::_buildHtml() const {
    // Calculer stats
    bool currentOn = (_count > 0) && _buf[(_head + _count - 1) % MAX_ENTRIES].isOn;
    uint8_t onCount = 0;
    for (uint8_t i = 0; i < _count; i++) {
        if (_buf[(_head + i) % MAX_ENTRIES].isOn)
            onCount++;
    }

    String html;
    html.reserve(3200);

    html +=
        F("<!DOCTYPE html><html lang='fr'><head>"
          "<meta charset='UTF-8'>"
          "<meta name='viewport' content='width=device-width,initial-scale=1'>"
          "<meta http-equiv='refresh' content='30'>"
          "<title>Moule - Contacts</title>"
          "<style>"
          "body{margin:0;padding:16px;font-family:sans-serif;"
          "background:#1a1a2e;color:#e0e0e0}"
          "h1{color:#e94560;margin:0 0 4px}"
          "p.sub{color:#888;font-size:.85em;margin:0 0 12px}"
          ".badge{display:inline-block;padding:3px 14px;"
          "border-radius:12px;font-weight:bold}"
          ".on{background:#1a4a2a;color:#2ecc71}"
          ".off{background:#4a1a1a;color:#e74c3c}"
          ".unk{background:#333;color:#aaa}"
          ".state{margin:12px 0;font-size:1.1em}"
          "table{width:100%;border-collapse:collapse;margin-top:16px}"
          "th{background:#0f3460;padding:9px 12px;text-align:left;"
          "border-bottom:2px solid #e94560;font-size:.9em}"
          "td{padding:8px 12px;border-bottom:1px solid #2a2a3e;font-size:.9em}"
          "tr:hover td{background:#16213e}"
          "p.foot{color:#666;font-size:.8em;margin-top:12px}"
          "a.btn{display:inline-block;margin-top:8px;padding:6px 16px;"
          "background:#e94560;color:#fff;border-radius:6px;"
          "text-decoration:none;font-size:.85em}"
          "a.btn:hover{background:#c73652}"
          "</style></head><body>");

    html += F("<h1>Moul&#233; &#8212; Journal des contacts</h1>");
    html += F("<p class='sub'>Rafra&#238;chissement automatique toutes les 30&nbsp;s</p>");

    // Badge état actuel
    html += F("<div class='state'>&#201;tat actuel&nbsp;: ");
    if (_count == 0) {
        html += F("<span class='badge unk'>Inconnu</span>");
    } else if (currentOn) {
        html += F("<span class='badge on'>&#9679;&nbsp;ON</span>");
    } else {
        html += F("<span class='badge off'>&#9679;&nbsp;OFF</span>");
    }
    html += F("</div>");

    // Tableau — du plus récent au plus ancien
    html +=
        F("<table>"
          "<tr><th>#</th><th>Date / Heure</th><th>&#201;tat</th><th>Dur&#233;e active</th></tr>");

    if (_count == 0) {
        html +=
            F("<tr><td colspan='4' style='text-align:center;color:#555'>"
              "Aucun &#233;v&#233;nement enregistr&#233;</td></tr>");
    } else {
        const time_t now = time(nullptr);
        for (int i = (int)_count - 1; i >= 0; i--) {
            uint8_t idx = (_head + (uint8_t)i) % MAX_ENTRIES;
            const LogEntry& e = _buf[idx];
            html += F("<tr><td>");
            html += String(i + 1);  // numéro chronologique (1 = plus ancien)
            html += F("</td><td>");
            html += _fmtTs(e.ts);
            html += F("</td><td>");
            if (e.isOn) {
                html += F("<span class='badge on'>ON</span>");
            } else {
                html += F("<span class='badge off'>OFF</span>");
            }
            html += F("</td><td>");
            if (e.isOn) {
                // Entrée ON : durée = jusqu'au OFF suivant, ou "en cours" si c'est la dernière
                if (i + 1 < (int)_count) {
                    const LogEntry& next = _buf[(_head + (uint8_t)(i + 1)) % MAX_ENTRIES];
                    if (!next.isOn && next.ts >= e.ts) {
                        html += _fmtDuration((uint32_t)(next.ts - e.ts));
                    } else {
                        html += F("&#8212;");
                    }
                } else {
                    // Contact encore actif — afficher le temps écoulé
                    html += _fmtDuration((uint32_t)(now - e.ts));
                    html += F(" <span style='color:#aaa;font-size:.8em'>(en cours)</span>");
                }
            } else {
                html += F("&#8212;");
            }
            html += F("</td></tr>");
        }
    }

    html += F("</table>");
    html += F("<p class='foot'>");
    html += String(_count);
    html += F("&nbsp;&#233;v&#233;nement(s) enregistr&#233;(s) (max&nbsp;100)&nbsp;&#8212;&nbsp;");
    html += String(onCount);
    html += F("&nbsp;activation(s)</p>");
    html += F("<a class='btn' href='/clear'>Effacer le journal</a>");
    html += F("</body></html>");

    return html;
}
