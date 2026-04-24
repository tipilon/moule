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
    _srv.on("/status", [this]() { _handleStatus(); });
    _srv.on("/update", HTTP_GET, [this]() { _handleUpdateGet(); });
    _srv.on(
        "/update", HTTP_POST, [this]() { _handleUpdatePost(); },
        [this]() {
            HTTPUpload& upload = _srv.upload();
            if (upload.status == UPLOAD_FILE_START) {
                Serial.printf("[OTA] Début : %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                }
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                esp_task_wdt_reset();  // empêche le watchdog de couper pendant le flash
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    Update.printError(Serial);
                }
            } else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) {
                    Serial.printf("[OTA] Succès : %u octets\n", upload.totalSize);
                } else {
                    Update.printError(Serial);
                }
            }
        });
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

// ── _handleStatus() ──────────────────────────────────────────
void ContactLog::_handleStatus() {
    _srv.send(200, "text/html; charset=utf-8", _buildStatusHtml());
}

// ── _handleUpdateGet() ───────────────────────────────────────
void ContactLog::_handleUpdateGet() {
    _srv.send(200, "text/html; charset=utf-8", _buildUpdateHtml());
}

// ── _handleUpdatePost() ──────────────────────────────────────
void ContactLog::_handleUpdatePost() {
    const bool ok = !Update.hasError();
    _srv.sendHeader("Connection", "close");
    _srv.send(200, "text/html; charset=utf-8",
              _buildUpdateHtml(ok ? "" : Update.errorString(), ok));
    if (ok) {
        delay(1000);
        ESP.restart();
    }
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
    if (sec == 0)
        return F("&lt;1s");
    char buf[16];
    if (sec < 60) {
        snprintf(buf, sizeof(buf), "%lus", (unsigned long) sec);
    } else if (sec < 3600) {
        snprintf(buf, sizeof(buf), "%lum%02lus", (unsigned long) (sec / 60),
                 (unsigned long) (sec % 60));
    } else {
        snprintf(buf, sizeof(buf), "%luh%02lum", (unsigned long) (sec / 3600),
                 (unsigned long) ((sec % 3600) / 60));
    }
    return String(buf);
}

// ── _fmtUptime() ─────────────────────────────────────────────
String ContactLog::_fmtUptime(uint32_t ms) const {
    uint32_t s = ms / 1000;
    char buf[32];
    if (s < 60) {
        snprintf(buf, sizeof(buf), "%lus", (unsigned long) s);
    } else if (s < 3600) {
        snprintf(buf, sizeof(buf), "%lum %lus", (unsigned long) (s / 60), (unsigned long) (s % 60));
    } else {
        snprintf(buf, sizeof(buf), "%luh %lum", (unsigned long) (s / 3600),
                 (unsigned long) ((s % 3600) / 60));
    }
    return String(buf);
}

// ── _buildUpdateHtml() ───────────────────────────────────────
String ContactLog::_buildUpdateHtml(const String& msg, bool success) const {
    String html;
    html.reserve(1024);
    html +=
        F("<!DOCTYPE html><html lang='fr'><head>"
          "<meta charset='UTF-8'>"
          "<meta name='viewport' content='width=device-width,initial-scale=1'>"
          "<title>Moule - Mise à jour</title>"
          "<style>"
          "body{margin:0;padding:16px;font-family:sans-serif;"
          "background:#1a1a2e;color:#e0e0e0}"
          "h1{color:#e94560;margin:0 0 4px}"
          "p.sub{color:#888;font-size:.85em;margin:0 0 20px}"
          ".box{background:#16213e;border-radius:8px;padding:20px;max-width:480px}"
          "input[type=file]{color:#e0e0e0;margin:12px 0;width:100%}"
          "button{background:#e94560;color:#fff;border:none;padding:10px 24px;"
          "border-radius:6px;font-size:1em;cursor:pointer;margin-top:8px}"
          "button:hover{background:#c73652}"
          ".ok{color:#2ecc71;margin-top:12px;font-weight:bold}"
          ".err{color:#e74c3c;margin-top:12px;font-weight:bold}"
          "a.btn{display:inline-block;margin-top:16px;padding:6px 16px;"
          "background:#0f3460;color:#fff;border-radius:6px;"
          "text-decoration:none;font-size:.85em}"
          "</style></head><body>"
          "<h1>Moul&#233; &#8212; Mise &#224; jour firmware</h1>"
          "<p class='sub'>S&#233;lectionnez le fichier <strong>.bin</strong> "
          "g&#233;n&#233;r&#233; par PlatformIO</p>"
          "<div class='box'>");

    if (success) {
        html += F("<p class='ok'>&#10003; Mise &#224; jour r&#233;ussie — red&#233;marrage...</p>");
    } else if (msg.length() > 0) {
        html += F("<p class='err'>&#10007; Erreur : ");
        html += msg;
        html += F("</p>");
    }

    html +=
        F("<form method='POST' enctype='multipart/form-data'>"
          "<input type='file' name='firmware' accept='.bin'><br>"
          "<button type='submit'>&#x1F4E4; Flasher</button>"
          "</form>"
          "<p style='color:#555;font-size:.8em;margin-top:12px'>"
          "Fichier : <code>.pio/build/esp32dev/firmware.bin</code></p>"
          "</div>"
          "<a class='btn' href='/status'>&#8592; Tableau de bord</a>"
          "</body></html>");
    return html;
}

// ── _buildStatusHtml() ───────────────────────────────────────
String ContactLog::_buildStatusHtml() const {
    const uint32_t now = millis();
    const bool ntpOk = (time(nullptr) > 1000000000UL);
    const int rssi = WiFi.RSSI();

    // Qualité signal : excellent > -60, bon > -70, faible > -80, mauvais
    const char* rssiLabel = (rssi > -60)   ? "Excellent"
                            : (rssi > -70) ? "Bon"
                            : (rssi > -80) ? "Faible"
                                           : "Mauvais";
    const char* rssiColor = (rssi > -60)   ? "#2ecc71"
                            : (rssi > -70) ? "#f39c12"
                            : (rssi > -80) ? "#e67e22"
                                           : "#e74c3c";

    String html;
    html.reserve(2048);

    html +=
        F("<!DOCTYPE html><html lang='fr'><head>"
          "<meta charset='UTF-8'>"
          "<meta name='viewport' content='width=device-width,initial-scale=1'>"
          "<meta http-equiv='refresh' content='10'>"
          "<title>Moule - Status</title>"
          "<style>"
          "body{margin:0;padding:16px;font-family:sans-serif;"
          "background:#1a1a2e;color:#e0e0e0}"
          "h1{color:#e94560;margin:0 0 4px}"
          "p.sub{color:#888;font-size:.85em;margin:0 0 16px}"
          "h2{color:#e94560;font-size:1em;margin:20px 0 8px;border-bottom:1px solid "
          "#2a2a3e;padding-bottom:4px}"
          "table{width:100%;border-collapse:collapse;margin-bottom:8px}"
          "td{padding:7px 10px;border-bottom:1px solid #2a2a3e;font-size:.9em}"
          "td:first-child{color:#888;width:45%}"
          "td:last-child{font-weight:bold}"
          "a.btn{display:inline-block;margin-top:12px;padding:6px 16px;"
          "background:#0f3460;color:#fff;border-radius:6px;"
          "text-decoration:none;font-size:.85em}"
          "a.btn:hover{background:#16213e}"
          "</style></head><body>");

    html += F("<h1>Moul&#233; &#8212; Tableau de bord</h1>");
    html += F("<p class='sub'>Rafra&#238;chissement automatique toutes les 10&nbsp;s</p>");

    // ── Système ──
    html += F("<h2>&#x2699; Syst&#232;me</h2><table>");
    html += F("<tr><td>Uptime</td><td>");
    html += _fmtUptime(now);
    html += F("</td></tr>");
    html += F("<tr><td>M&#233;moire libre (heap)</td><td>");
    html += String(ESP.getFreeHeap() / 1024);
    html += F("&nbsp;KB</td></tr>");
    html += F("<tr><td>Chip</td><td>");
    html += String(ESP.getChipModel());
    html += F(" rev ");
    html += String(ESP.getChipRevision());
    html += F("</td></tr>");
    html += F("<tr><td>CPU</td><td>");
    html += String(ESP.getCpuFreqMHz());
    html += F("&nbsp;MHz</td></tr>");
    html += F("<tr><td>Flash</td><td>");
    html += String(ESP.getFlashChipSize() / (1024 * 1024));
    html += F("&nbsp;MB</td></tr>");
    html += F("</table>");

    // ── WiFi ──
    html += F("<h2>&#x1F4F6; WiFi</h2><table>");
    html += F("<tr><td>Adresse IP</td><td>");
    html += WiFi.localIP().toString();
    html += F("</td></tr>");
    html += F("<tr><td>Hostname</td><td>");
    html += WiFi.getHostname();
    html += F("</td></tr>");
    html += F("<tr><td>SSID</td><td>");
    html += WiFi.SSID();
    html += F("</td></tr>");
    html += F("<tr><td>Signal (RSSI)</td><td style='color:");
    html += rssiColor;
    html += F("'>");
    html += String(rssi);
    html += F("&nbsp;dBm &mdash; ");
    html += rssiLabel;
    html += F("</td></tr>");
    html += F("</table>");

    // ── NTP ──
    html += F("<h2>&#x1F552; Horloge NTP</h2><table>");
    html += F("<tr><td>Synchronis&#233;</td><td style='color:");
    html += ntpOk ? F("#2ecc71'>Oui") : F("#e74c3c'>Non");
    html += F("</td></tr>");
    if (ntpOk) {
        html += F("<tr><td>Heure actuelle</td><td>");
        html += _fmtTs(time(nullptr));
        html += F("</td></tr>");
    }
    html += F("</table>");

    // ── Journal ──
    html += F("<h2>&#x1F4CB; Journal</h2><table>");
    html += F("<tr><td>&#201;v&#233;nements enregistr&#233;s</td><td>");
    html += String(_count);
    html += F("&nbsp;/ 100</td></tr></table>");

    html += F("<a class='btn' href='/'>&#8592; Journal des contacts</a>");
    html += F("</body></html>");

    return html;
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
          ".over{color:#f39c12;font-weight:bold}"
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
        for (int i = (int) _count - 1; i >= 0; i--) {
            uint8_t idx = (_head + (uint8_t) i) % MAX_ENTRIES;
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
                if (i + 1 < (int) _count) {
                    const LogEntry& next = _buf[(_head + (uint8_t) (i + 1)) % MAX_ENTRIES];
                    if (!next.isOn && next.ts >= e.ts) {
                        const uint32_t durSec = (uint32_t) (next.ts - e.ts);
                        const bool over = (durSec * 1000UL >= PALETTE_TIMEOUT_MS);
                        if (over)
                            html += F("<span class='over'>");
                        html += _fmtDuration(durSec);
                        if (over)
                            html += F("</span>");
                    } else {
                        html += F("&#8212;");
                    }
                } else {
                    // Contact encore actif — afficher le temps écoulé
                    const uint32_t durSec = (uint32_t) (now - e.ts);
                    const bool over = (durSec * 1000UL >= PALETTE_TIMEOUT_MS);
                    if (over)
                        html += F("<span class='over'>");
                    html += _fmtDuration(durSec);
                    if (over)
                        html += F("</span>");
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
