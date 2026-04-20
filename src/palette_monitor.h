// ============================================================
//  palette_monitor.h — moulé
//  Surveillance du signal palette (opto-isolateur)
//
//  Comportement :
//    - Front montant sur PALETTE_PIN  → démarre le décompte
//    - Délai PALETTE_TIMEOUT_MS dépassé → allume PALETTE_LIGHT_PIN
//    - Front descendant sur PALETTE_PIN → éteint la lumière
//
//  Machine à états : IDLE ─► TIMING ─► ALARM ─► IDLE
// ============================================================

#pragma once
#include <Arduino.h>

#include <functional>

// ── États internes ────────────────────────────────────────────
enum class PaletteState : uint8_t {
    IDLE,    // Signal bas  — en attente
    TIMING,  // Signal haut — décompte en cours, lumière éteinte
    ALARM    // Délai dépassé — lumière allumée
};

// ── Classe PaletteMonitor ─────────────────────────────────────
class PaletteMonitor {
   public:
    // inputPin  : GPIO branché à la sortie de l'opto-isolateur
    // lightPin  : GPIO relié à la lumière d'alarme (relais ou LED)
    // timeoutMs : délai max autorisé avant alarme, en ms
    void begin(uint8_t inputPin, uint8_t lightPin, uint32_t timeoutMs);

    // À appeler à chaque itération de loop() — entièrement non-bloquant
    void update();

    // Callback : changement d'état signal (true=front montant, false=front descendant)
    void setOnContact(std::function<void(bool)> cb) { _onContact = cb; }

    // Callback : déclenché au passage TIMING → ALARM (délai dépassé)
    void setOnAlarm(std::function<void()> cb) { _onAlarm = cb; }

    // ── Accesseurs ───────────────────────────────────────────
    PaletteState getState() const { return _state; }
    bool isPaletteActive() const { return _debouncedHigh; }
    bool isAlarmOn() const { return _state == PaletteState::ALARM; }

    // Millisecondes écoulées depuis le front montant (0 si IDLE)
    uint32_t getElapsedMs() const;

   private:
    uint8_t _inputPin;
    uint8_t _lightPin;
    uint32_t _timeoutMs;

    PaletteState _state;
    bool _debouncedHigh;  // état logique stable, après debounce
    bool _lastRaw;        // dernière lecture brute du GPIO

    bool _debouncing;         // une transition est en cours de validation
    bool _pendingState;       // état candidat pendant la fenêtre debounce
    uint32_t _debounceStart;  // timestamp du début du debounce

    uint32_t _risingEdgeTime;  // millis() au moment du front montant

    std::function<void(bool)> _onContact;  // callback signal ON/OFF (optionnel)
    std::function<void()> _onAlarm;        // callback alarme déclenchée (optionnel)

    void _setLight(bool on);
};
