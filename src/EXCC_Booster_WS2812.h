#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "EXCC_Config.h"

class BoosterTelemetry;   // Forward declaration

class EXCC_Booster_WS2812
{
public:
    EXCC_Booster_WS2812();

    void begin();
    void update();
    void setEnabled(bool enabled);

    void onCanMessage(uint32_t id, const uint8_t *data, uint8_t len);

    void onCutoutStart();
    void onCutoutEnd();
    void feedRailcomSample();

    uint16_t readCurrent_mA() const;
    uint16_t readVoltage_mV() const;
    bool isThermalFault() const;

    // Getter utilisé par EXCC_StatusLed
    const BoosterTelemetry& getTelemetry() const;

    // <<< AJOUT ESSENTIEL POUR LED 1
    bool isEnabled() const { return m_enabled; }

private:
    void updateOccupation(uint16_t courant_mA);
    void updateLedState();   // sera supprimée après migration LED 1

private:
    bool m_enabled;
    bool m_lastOccupe;

    uint16_t m_current_mA;
    uint16_t m_voltage_mV;
    bool m_faultThermal;
};
