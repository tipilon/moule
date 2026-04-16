// ============================================================
//  palette_monitor.cpp — moulé
//  Voir palette_monitor.h pour la description complète.
// ============================================================

#include "palette_monitor.h"
#include "config.h"

// ── begin() ──────────────────────────────────────────────────
void PaletteMonitor::begin(uint8_t inputPin, uint8_t lightPin, uint32_t timeoutMs) {
    _inputPin   = inputPin;
    _lightPin   = lightPin;
    _timeoutMs  = timeoutMs;

    _state          = PaletteState::IDLE;
    _debouncedHigh  = false;
    _lastRaw        = false;
    _debouncing     = false;
    _pendingState   = false;
    _debounceStart  = 0;
    _risingEdgeTime = 0;

    // Mode pull selon polarité active configurée dans config.h
#if PALETTE_ACTIVE_HIGH
    // Opto-isolateur ou switch → HIGH quand actif : pull-down interne
    pinMode(_inputPin, INPUT_PULLDOWN);
#else
    // Opto-isolateur NPN typique : collecteur ouvert → LOW quand actif : pull-up interne
    pinMode(_inputPin, INPUT_PULLUP);
#endif

    pinMode(_lightPin, OUTPUT);
    _setLight(false);   // s'assure que la lumière est éteinte au démarrage

    Serial.printf("[Palette] init — entrée GPIO%u  lumière GPIO%u  délai %ums\n",
                  _inputPin, _lightPin, _timeoutMs);
}

// ── update() — appelé dans loop() ────────────────────────────
void PaletteMonitor::update() {
    const uint32_t now    = millis();
    const bool     rawPin = (digitalRead(_inputPin) == HIGH);

    // ── Debounce anti-rebond ──────────────────────────────────
    if (rawPin != _lastRaw) {
        // Un front est détecté — démarrer (ou relancer) la fenêtre de debounce
        _lastRaw       = rawPin;
        _debouncing    = true;
        _pendingState  = rawPin;
        _debounceStart = now;
    }

    if (_debouncing && (now - _debounceStart >= PALETTE_DEBOUNCE_MS)) {
        // La fenêtre de debounce est écoulée — valider la transition
        _debouncing = false;

        if (_pendingState != _debouncedHigh) {
            const bool rising  = _pendingState  && !_debouncedHigh;
            const bool falling = !_pendingState &&  _debouncedHigh;
            _debouncedHigh = _pendingState;

            if (rising) {
                // ── Front montant : démarrer le décompte ─────
                _risingEdgeTime = now;
                _state = PaletteState::TIMING;
                Serial.println("[Palette] ↑ front montant — décompte démarré");
                if (_onContact) _onContact(true);
            } else if (falling) {
                // ── Front descendant : reset alarme ──────────
                const uint32_t duration = now - _risingEdgeTime;
                Serial.printf("[Palette] ↓ front descendant — durée active %ums\n", duration);
                _state = PaletteState::IDLE;
                _setLight(false);
                if (_onContact) _onContact(false);
            }
        }
    }

    // ── Transition TIMING → ALARM ─────────────────────────────
    if (_state == PaletteState::TIMING) {
        if ((now - _risingEdgeTime) >= _timeoutMs) {
            _state = PaletteState::ALARM;
            _setLight(true);
            Serial.printf("[Palette] *** ALARME *** délai %ums dépassé !\n", _timeoutMs);
        }
    }
}

// ── getElapsedMs() ────────────────────────────────────────────
uint32_t PaletteMonitor::getElapsedMs() const {
    if (_state == PaletteState::IDLE) return 0;
    return millis() - _risingEdgeTime;
}

// ── _setLight() ───────────────────────────────────────────────
void PaletteMonitor::_setLight(bool on) {
#if PALETTE_LIGHT_ACTIVE_HIGH
    digitalWrite(_lightPin, on ? HIGH : LOW);
#else
    // Relais actif-bas (ex : module relais 5 V à logique inversée)
    digitalWrite(_lightPin, on ? LOW : HIGH);
#endif
}
