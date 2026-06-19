#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

/*
 * ============================================================================
 *  EXCC_System — Version WS2812 (Gestion Canton 2026)
 * ----------------------------------------------------------------------------
 *  Rôle :
 *    Point d’entrée de l’initialisation complète du système EXCC.
 *
 *    Cette version initialise :
 *      • UART RS485 (réception CC → EXCC)
 *      • Servos (PCA9685)
 *      • Canton WS2812 (occupation + mouvement + erreur)
 *      • Micro‑switchs d’aiguilles
 *      • Signaux SNCF WS2812 (H et AH)
 *      • Feux directionnels WS2812 (H et AH)
 *      • Booster + Calibration
 *
 *  Notes :
 *    - Le PCA9685 ne pilote plus aucune LED.
 *    - Le canton est désormais 100 % WS2812.
 *    - Les feux directionnels sont WS2812/WS2815.
 *    - Le booster reste inchangé.
 * ============================================================================
 */

namespace EXCC_System
{
    // Initialisation complète du système EXCC
    void init() noexcept;
}
