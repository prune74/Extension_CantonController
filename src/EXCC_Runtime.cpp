/*
 * ============================================================================
 *  Module : EXCC_Runtime (WS2812 Version)
 *  Rôle   : Boucle d’exécution principale EXCC (hors Booster).
 *
 *  Fonctionnement général (Gestion Canton 2026) :
 *    - Appelé en continu par EXCC_Main::loop()
 *    - Exécute toutes les tâches temps réel EXCC :
 *         1) Mise à jour signaux SNCF WS2812 (H et AH)
 *         2) Mise à jour canton WS2812 (occupation + mouvement + erreur)
 *         3) Mise à jour aiguilles (anti‑blocage)
 *
 *  Notes :
 *    - Le Booster tourne dans sa propre tâche FreeRTOS.
 *    - Les feux directionnels WS2812 n’ont pas de logique interne :
 *         → pas de clignotement
 *         → pas d’animation
 *         → pas de mise à jour dans la boucle runtime
 * ============================================================================
 */

#include "EXCC_Runtime.h"

#include "EXCC_Signaux_WS2812.h"
#include "EXCC_Canton_WS2812.h"
#include "EXCC_Switches.h"

#include <FastLED.h>
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

// Instances globales déclarées dans EXCC_Main.cpp
extern EXCC_Signaux_WS2812 signauxH;
extern EXCC_Signaux_WS2812 signauxAH;

extern EXCC_Canton_WS2812 cantonWS;

/* ============================================================================
 * update()
 * ----------------------------------------------------------------------------
 * Boucle principale EXCC (hors booster).
 * Appelée en continu par EXCC_Main::loop().
 * ============================================================================
 */
void EXCC_Runtime::update() noexcept
{
    uint32_t now = millis();

    /* --------------------------------------------------------
       1) Mise à jour des signaux SNCF WS2812
       -------------------------------------------------------- */
    signauxH.update(now);
    signauxAH.update(now);

    /* --------------------------------------------------------
       2) Mise à jour canton WS2812
       -------------------------------------------------------- */
    cantonWS.update();

    /* --------------------------------------------------------
       3) Mise à jour aiguilles (anti‑blocage)
       -------------------------------------------------------- */
    EXCC_Switches::update();

    /* --------------------------------------------------------
       4) Application WS2812
       -------------------------------------------------------- */
    FastLED.show();
}
