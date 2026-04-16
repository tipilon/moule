// ============================================================
//  contact_log.h — moulé
//  Journal des événements de contact (ON/OFF)
//
//  Fonctionnalités :
//    - Tampon circulaire de 100 événements horodatés (NTP)
//    - Serveur HTTP sur port 80 — page consultable par navigateur
//    - Endpoint /clear pour vider le journal
// ============================================================

#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <time.h>

class ContactLog {
   public:
    // À appeler après que le WiFi soit connecté
    void begin();

    // À appeler dans loop() — traite les requêtes HTTP entrantes
    void handle();

    // Enregistre un événement : isOn=true → contact fermé, false → ouvert
    void addEvent(bool isOn);

   private:
    struct LogEntry {
        time_t ts;  // Unix timestamp (epoch) ; < 1 000 000 000 si NTP non synchro
        bool isOn;
    };

    static constexpr uint8_t MAX_ENTRIES = 100;

    LogEntry _buf[MAX_ENTRIES];
    uint8_t _head = 0;   // index de l'entrée la plus ancienne
    uint8_t _count = 0;  // nombre d'entrées valides (0..MAX_ENTRIES)

    WebServer _srv{80};

    // Gestionnaires HTTP
    void _handleRoot();
    void _handleClear();

    // Construction de la page HTML
    String _buildHtml() const;
    String _fmtTs(time_t t) const;
};
