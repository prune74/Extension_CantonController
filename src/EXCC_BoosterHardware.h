#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "CanDccBoosterHardware.h"

class EXCC_BoosterHardware : public CanDccBoosterHardware
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
    int16_t readRailcomSample() override;

private:
    void setupDrv8874();
    void setupPwmDcc();
    void setupAdc();
};
