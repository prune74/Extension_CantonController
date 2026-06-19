#include "EXCC_Booster_WS2812.h"

#include "EXCC_BoosterHardware.h"
#include "CanDccBooster.h"
#include "EXCC_CanBooster.h"
#include "EXCC_Calibration.h"
#include "EXCC_UartTx.h"
#include "EXCC_Canton_WS2812.h"

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

// ---------------------------------------------------------------------------
//  Instances internes
// ---------------------------------------------------------------------------
static EXCC_BoosterHardware s_hw;
static BoosterConfig s_cfg;
static CanDccBooster s_booster(s_hw, s_cfg);

// Canton (LED 0)
extern EXCC_Canton_WS2812 cantonWS;

// ---------------------------------------------------------------------------
//  Constructeur
// ---------------------------------------------------------------------------
EXCC_Booster_WS2812::EXCC_Booster_WS2812()
    : m_enabled(true),
      m_lastOccupe(false),
      m_current_mA(0),
      m_voltage_mV(0),
      m_faultThermal(false)
{
}

// ---------------------------------------------------------------------------
//  begin()
// ---------------------------------------------------------------------------
void EXCC_Booster_WS2812::begin()
{
    s_hw.begin();

    s_cfg.maxCurrent_mA   = EXCC_BOOSTER_MAX_COURANT_mA;
    s_cfg.minVoltage_mV   = EXCC_BOOSTER_MIN_TENSION_mV;
    s_cfg.telemetryEnabled = true;
    s_cfg.railcomEnabled   = true;

    s_booster.setConfig(s_cfg);
}

// ---------------------------------------------------------------------------
//  setEnabled()
// ---------------------------------------------------------------------------
void EXCC_Booster_WS2812::setEnabled(bool enabled)
{
    m_enabled = enabled;

    if (!enabled)
    {
        s_hw.disableOutput();

        m_current_mA   = 0;
        m_voltage_mV   = 0;
        m_faultThermal = false;
        m_lastOccupe   = false;

        cantonWS.setOccupation(false);
    }
}

// ---------------------------------------------------------------------------
//  update()
// ---------------------------------------------------------------------------
void EXCC_Booster_WS2812::update()
{
    if (!m_enabled)
    {
        s_hw.disableOutput();
        return;
    }

    s_booster.update();

    const BoosterTelemetry &t = s_booster.getTelemetry();
    m_current_mA   = t.current_mA;
    m_voltage_mV   = t.voltage_mV;
    m_faultThermal = (t.error == BoosterError::HARDWARE_FAULT);

    if (t.railcomAddress != BoosterConstants::RAILCOM_NO_ADDRESS)
        EXCC_UartTx::envoyerRailcom(0, t.railcomAddress);

    updateOccupation(m_current_mA);
}

// ---------------------------------------------------------------------------
//  onCanMessage()
// ---------------------------------------------------------------------------
void EXCC_Booster_WS2812::onCanMessage(uint32_t id, const uint8_t *data, uint8_t len)
{
    s_booster.onCanMessage(id, data, len);
}

// ---------------------------------------------------------------------------
//  RailCom hooks
// ---------------------------------------------------------------------------
void EXCC_Booster_WS2812::onCutoutStart()
{
    s_booster.onCutoutStart();
}

void EXCC_Booster_WS2812::onCutoutEnd()
{
    s_booster.onCutoutEnd();
}

void EXCC_Booster_WS2812::feedRailcomSample()
{
    s_booster.feedRailcomSample();
}

// ---------------------------------------------------------------------------
//  Occupation
// ---------------------------------------------------------------------------
void EXCC_Booster_WS2812::updateOccupation(uint16_t courant_mA)
{
    uint16_t seuilLibre  = EXCC_Calibration::getSeuilLibre();
    uint16_t seuilOccupe = EXCC_Calibration::getSeuilOccupe();

    bool occupe;

    if (m_lastOccupe)
        occupe = (courant_mA > seuilLibre);
    else
        occupe = (courant_mA > seuilOccupe);

    if (occupe != m_lastOccupe)
    {
        m_lastOccupe = occupe;
        EXCC_UartTx::envoyerOccupation(occupe);
        cantonWS.setOccupation(occupe);
    }
}

// ---------------------------------------------------------------------------
//  Accesseurs
// ---------------------------------------------------------------------------
uint16_t EXCC_Booster_WS2812::readCurrent_mA() const { return m_current_mA; }
uint16_t EXCC_Booster_WS2812::readVoltage_mV() const { return m_voltage_mV; }
bool EXCC_Booster_WS2812::isThermalFault() const { return m_faultThermal; }

const BoosterTelemetry &EXCC_Booster_WS2812::getTelemetry() const
{
    return s_booster.getTelemetry();
}
