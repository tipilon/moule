// ============================================================
//  notifier.h — moulé
//  Envoi de notifications push via ntfy.sh
//
//  Utilisation :
//    notifier.begin();
//    notifier.sendAlert("Cycle trop long — vérifier la moulée");
//    notifier.sendResolved("Cycle revenu à la normale");
// ============================================================

#pragma once
#include <Arduino.h>

class Notifier {
   public:
    void begin();

    // Envoie une notification d'alarme (priorité urgente, icône ⚠)
    bool sendAlert(const String& message);

    // Envoie une notification de retour à la normale (priorité normale, icône ✓)
    bool sendResolved(const String& message);

   private:
    bool _post(const String& message, const char* title, const char* priority, const char* tags);
};
