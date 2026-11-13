#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include <stddef.h>
#include "EXCC_Config.h"
#include "EXCC_DccRegen.h"
#include "CAN_DCC_RMT-Booster_Hardware.h"

/* ============================================================
 * EXCC_BoosterHardware — PHASE 2 : régénération RMT canal 7
 *
 * IN1 (GPIO16) = RMT TX canal 7, IN2 (GPIO17) = même signal
 * INVERSÉ par la matrice GPIO. DRV8874 mode PWM (PMODE=1) :
 *   Cutout   : IN détachés du RMT, forcé 1/1 → BRAKE
 *   Failsafe : IN détachés, forcé 0/0 → COAST (Hi-Z)
 * ============================================================ */
class EXCC_BoosterHardware : public CanDccRmtBoosterHardware
{
public:
    void begin();

    void applyDcc(const uint8_t *data, uint8_t len) override;
    size_t emitDccBits(const uint8_t *bits, size_t count);
    bool isDccBusy();
    void enableCutout() override;
    void disableCutout() override;

    void enableOutput() override;
    void disableOutput() override;

    bool isFaultActive() override;

    uint16_t readCurrent_mA() override;
    uint16_t readVoltage_mV() override;
    int16_t readRailcomSample() override; // obsolète Phase 2

    // Sonde bench/diagnostic : bits DCC passés au RMT depuis le boot
    static uint32_t bitsEmis() { return s_bitsEmis; }

private:
    void setupDrv8874();
    void connectDcc();
    void setupAdc();

    static uint32_t s_bitsEmis;

    EXCC_DccRegen m_regen;
    bool m_connected = false;
    bool m_braking = false;
};