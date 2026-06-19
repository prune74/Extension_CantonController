#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

/*
 * ============================================================================
 *  EXCC_Runtime — Version WS2812 (Gestion Canton 2026)
 * ----------------------------------------------------------------------------
 *  Rôle :
 *    Boucle d’exécution principale EXCC (hors Booster).
 *
 *    Mise à jour temps réel :
 *      • Signaux SNCF WS2812 (H et AH)
 *      • Canton WS2812 (occupation + mouvement + erreur)
 *      • Aiguilles (anti‑blocage)
 *
 *  Notes :
 *    - Le Booster tourne dans sa propre tâche FreeRTOS.
 *    - Les feux directionnels WS2812 n’ont aucune logique interne :
 *         → pas de clignotement
 *         → pas d’animation
 *         → pas de mise à jour dans la boucle runtime
 *
 *  Appelé en continu par EXCC_Main::loop()
 * ============================================================================
 */

namespace EXCC_Runtime
{
    // Boucle principale EXCC (hors Booster)
    void update() noexcept;
}
