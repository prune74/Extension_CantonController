#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

/*
 * ============================================================================
 *  EXCC_Runtime (Chemin de Fer du Chablais (CFC))
 * ----------------------------------------------------------------------------
 *  Rôle :
 *    Boucle d’exécution principale EXCC et passerelles CAN/UART.
 *
 *    Mise à jour temps réel :
 *      • Signaux SNCF WS2812 (H et AH)
 *      • Canton WS2812 (occupation + mouvement + erreur)
 *      • Aiguilles (anti‑blocage)
 *
 *  Notes :
 *    - Le Booster externe est supervisé par UART.
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
    // Boucle principale EXCC et passerelles CAN/UART
    void update() noexcept;
}
