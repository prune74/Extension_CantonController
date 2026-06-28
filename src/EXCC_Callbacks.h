#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "EXCC_Config.h"

/*
 * ============================================================================
 *  EXCC_Callbacks.h — Callbacks CC → EXCC
 *  Version WS2812 (Gestion Canton 2026)
 * ============================================================================
 */

namespace EXCC_Callbacks
{
    /* ------------------------------------------------------------------------
     * ID du canton (FD)
     * ------------------------------------------------------------------------ */
    void onCantonID(const uint8_t *data, uint8_t len) noexcept;

    /* ------------------------------------------------------------------------
     * PING (FC)
     * ------------------------------------------------------------------------ */
    void onPing() noexcept;

    /* ------------------------------------------------------------------------
     * Configuration signaux (F6)
     * ------------------------------------------------------------------------ */
    void onConfigSignaux(const uint8_t *data, uint8_t len) noexcept;

    /* ------------------------------------------------------------------------
     * Aspects SNCF (F7 / F8)
     * ------------------------------------------------------------------------ */
    void onAspectHoraire(uint8_t aspect) noexcept;
    void onAspectAntiHoraire(uint8_t aspect) noexcept;

    /* ------------------------------------------------------------------------
     * Directions LED (F9 / FA)
     * ------------------------------------------------------------------------ */
    void onDirectionHoraire(uint8_t code) noexcept;
    void onDirectionAntiHoraire(uint8_t code) noexcept;

    /* ------------------------------------------------------------------------
     * Occupation voisins (FB)
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

    /* ------------------------------------------------------------------------
     * Profil de voie 12V(N) ou 15V(HO)
     * ------------------------------------------------------------------------ */
    void onProfileVoie(uint8_t profile) noexcept;
}
