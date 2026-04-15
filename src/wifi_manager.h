#pragma once
// ============================================================
//  wifi_manager.h
//  Gestion de la connexion WiFi avec reconnexion automatique
// ============================================================

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager()  = default;
    ~WiFiManager() = default;

    // Interdit la copie (ressource réseau unique)
    WiFiManager(const WiFiManager&)            = delete;
    WiFiManager& operator=(const WiFiManager&) = delete;

    /**
     * @brief Initialise le WiFi. À appeler une fois dans setup().
     * @param ssid       SSID du réseau
     * @param password   Mot de passe
     * @param hostname   Nom mDNS/DHCP (optionnel)
     */
    void begin(const char* ssid, const char* password,
               const char* hostname = nullptr);

    /**
     * @brief Tente la connexion de manière bloquante.
     * @param timeoutMs  Délai maximum en millisecondes
     * @return true si connecté, false si timeout
     */
    bool connect(uint32_t timeoutMs = 15000U);

    /** @brief Déconnecte proprement. */
    void disconnect();

    /**
     * @brief À appeler dans loop() — gère la reconnexion automatique.
     */
    void maintain();

    /** @return true si le WiFi est connecté */
    bool isConnected() const;

    /** @return Adresse IP locale sous forme de String */
    String getLocalIP() const;

    /** @return Force du signal en dBm */
    int getRSSI() const;

    /** @return Nombre de reconnexions depuis le démarrage */
    uint32_t getReconnectCount() const { return _reconnectCount; }

private:
    const char* _ssid     = nullptr;
    const char* _password = nullptr;
    const char* _hostname = nullptr;

    uint32_t _lastReconnectAttempt = 0;
    uint32_t _reconnectCount       = 0;

    static constexpr uint32_t RECONNECT_DELAY_MS = 5000U;

    void _log(const char* level, const char* msg) const;
};
