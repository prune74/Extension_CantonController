#pragma once
#include <stdint.h>

/*
 * ============================================================================
 *  EXCC_Callbacks.h — Callbacks CC → EXCC
 *  Version WS2812 (Gestion Canton 2026)
 * ----------------------------------------------------------------------------
 *  Rôle :
 *    Point d’entrée unique pour toutes les commandes envoyées par le CC.
 *
 *    Cette version pilote :
 *      • Signaux SNCF WS2812 (H et AH)
 *      • Feux directionnels WS2812 (H et AH)
 *      • Canton WS2812 (occupation, mouvement, erreur)
 *      • Servos (PCA9685)
 *      • Switches (anti-blocage)
 *      • Booster / Calibration
 *
 *  Notes :
 *    - Aucun aspect SNCF n’est interprété ici : simple dispatch.
 *    - Aucun layout n’est défini ici : EXCC_Signaux_WS2812 s’en charge.
 *    - Aucun protocole n’est décodé ici : tout arrive déjà structuré.
 * ============================================================================
 */

namespace EXCC_Callbacks
{
    /* ------------------------------------------------------------------------
     * Topologie / Configuration
     * ------------------------------------------------------------------------ */
    void onTopologie(uint8_t *data, uint8_t len) noexcept;
    void onConfigSignaux(uint8_t *data, uint8_t len) noexcept;

    /* ------------------------------------------------------------------------
     * Aspects SNCF (H / AH)
     * ------------------------------------------------------------------------ */
    void onAspectHoraire(uint8_t aspect) noexcept;
    void onAspectAntiHoraire(uint8_t aspect) noexcept;

    /* ------------------------------------------------------------------------
     * Directions LED (H / AH)
     * ------------------------------------------------------------------------ */
    void onDirectionHoraire(uint8_t code) noexcept;
    void onDirectionAntiHoraire(uint8_t code) noexcept;

    /* ------------------------------------------------------------------------
     * Occupation voisins (EA)
     * ------------------------------------------------------------------------ */
    void onOccupationVoisins(uint8_t value) noexcept;

    /* ------------------------------------------------------------------------
     * Servos (F0 / F1 / F2)
     * ------------------------------------------------------------------------ */
    void onServoMove(uint8_t servoIndex, uint8_t direction) noexcept;
    void onServoConfig(uint8_t servoIndex,
                       uint16_t posDroit,
                       uint16_t posDevie,
                       uint16_t speed) noexcept;
    void onServoTest(uint8_t servoIndex) noexcept;

    /* ------------------------------------------------------------------------
     * Booster (F3 / F4 / F5)
     * ------------------------------------------------------------------------ */
    void onRecalibrationBooster() noexcept;
    void onSetSeuils(uint16_t libre, uint16_t occupe) noexcept;
    void onBoosterPower(uint8_t power) noexcept;
}
