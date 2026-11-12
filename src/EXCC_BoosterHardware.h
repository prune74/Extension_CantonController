#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "EXCC_Config.h"
#include "EXCC_DccRegen.h"
#include "CAN_DCC_RMT-Booster_Hardware.h"

/* ============================================================
 * EXCC_BoosterHardware — PHASE 2 : vraie régénération RMT
 *
 * IN1 (GPIO16) = RMT TX canal 7, IN2 (GPIO17) = même signal
 * INVERSÉ par la matrice GPIO (inv=true). Le DRV8874 (PMODE=1)
 * reçoit un DCC biphasé aux timings exacts 58/100 µs.
 *   Cutout   : IN détachés du RMT, forcé 1/1 → BRAKE
 *   Failsafe : IN détachés, forcé 0/0 → COAST (Hi-Z)
 * ============================================================ */
class EXCC_BoosterHardware : public CanDccRmtBoosterHardware
{
public:
    void begin();

    void applyDcc(const uint8_t *data, uint8_t len) override;
    void enableCutout() override;
    void disableCutout() override;

    void enableOutput() override;
    void disableOutput() override;

    bool isFaultActive() override;

    uint16_t readCurrent_mA() override;
    uint16_t readVoltage_mV() override;
    int16_t readRailcomSample() override; // obsolète Phase 2 : conservé pour l'interface

private:
    void setupDrv8874();
    void connectDcc();
    void setupAdc();

    EXCC_DccRegen m_regen;
    bool m_connected = false;
    bool m_braking = false;
};