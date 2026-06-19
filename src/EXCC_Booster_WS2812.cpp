#include "EXCC_Booster_WS2812.h"

#include "EXCC_BoosterHardware.h"
#include "CanDccBooster.h"
#include "EXCC_CanBooster.h"
#include "EXCC_Calibration.h"
#include "EXCC_Config.h"
#include "EXCC_UartTx.h"
#include "EXCC_Canton_WS2812.h"

#include <FastLED.h>
#include <Arduino.h>

// ---------------------------------------------------------------------------
//  Instances internes
// ---------------------------------------------------------------------------
static EXCC_BoosterHardware s_hw;
static BoosterConfig s_cfg;
static CanDccBooster s_booster(s_hw, s_cfg);

// LEDs WS2812 (déclarées dans EXCC_Main.cpp)
extern CRGB g_wsStrip[4];
extern EXCC_Canton_WS2812 cantonWS;

static CRGB *LED_STATE = &g_wsStrip[1];
static CRGB *LED_RAILCOM = &g_wsStrip[2];
static CRGB *LED_TELEM = &g_wsStrip[3];

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

    s_cfg.maxCurrent_mA = EXCC_BOOSTER_MAX_COURANT_mA;
    s_cfg.minVoltage_mV = EXCC_BOOSTER_MIN_TENSION_mV;
    s_cfg.telemetryEnabled = true;
    s_cfg.railcomEnabled = true;

    s_booster.setConfig(s_cfg);

    *LED_STATE = CRGB::Black;
    *LED_RAILCOM = CRGB::Black;
    *LED_TELEM = CRGB::Black;
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

        m_current_mA = 0;
        m_voltage_mV = 0;
        m_faultThermal = false;
        m_lastOccupe = false;

        cantonWS.setOccupation(false);

        *LED_STATE = CRGB::Red;
        *LED_RAILCOM = CRGB::Black;
        *LED_TELEM = CRGB::Black;
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
    m_current_mA = t.current_mA;
    m_voltage_mV = t.voltage_mV;
    m_faultThermal = (t.error == BoosterError::HARDWARE_FAULT);

    if (t.railcomAddress != BoosterConstants::RAILCOM_NO_ADDRESS)
        EXCC_UartTx::envoyerRailcom(0, t.railcomAddress);

    updateOccupation(m_current_mA);

    updateLedState();
    updateLedRailcom();
    updateLedTelemetry();
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
    *LED_RAILCOM = CRGB::White;
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
    uint16_t seuilLibre = EXCC_Calibration::getSeuilLibre();
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
//  LED 1 : état général
// ---------------------------------------------------------------------------
void EXCC_Booster_WS2812::updateLedState()
{
    ExccBoosterEtat etat;

    if (!m_enabled)
        etat = BOOSTER_OFF;
    else if (m_voltage_mV < EXCC_BOOSTER_MIN_TENSION_mV)
        etat = BOOSTER_SOUS_TENSION;
    else if (m_current_mA > EXCC_BOOSTER_MAX_COURANT_mA)
        etat = BOOSTER_COURT_CIRCUIT;
    else if (m_faultThermal)
        etat = BOOSTER_SURCHAUFFE;
    else
        etat = BOOSTER_OK;

    switch (etat)
    {
    case BOOSTER_OFF:
        *LED_STATE = CRGB::Red;
        break;

    case BOOSTER_OK:
        *LED_STATE = CRGB::Green;
        break;

    case BOOSTER_COURT_CIRCUIT:
        *LED_STATE = (millis() & 200) ? CRGB::Red : CRGB::Black;
        break;

    case BOOSTER_SOUS_TENSION:
        *LED_STATE = CRGB::Blue;
        break;

    case BOOSTER_SURCHAUFFE:
    {
        uint8_t p = sin8(millis() >> 3);
        *LED_STATE = CRGB(p, 0, p);
        break;
    }

    case BOOSTER_ERREUR:
        *LED_STATE = (millis() & 300) ? CRGB::Red : CRGB::Purple;
        break;
    }
}

// ---------------------------------------------------------------------------
//  LED 2 : RailCom
// ---------------------------------------------------------------------------
void EXCC_Booster_WS2812::updateLedRailcom()
{
    const BoosterTelemetry &t = s_booster.getTelemetry();

    if (t.railcomAddress != BoosterConstants::RAILCOM_NO_ADDRESS)
        *LED_RAILCOM = (millis() & 100) ? CRGB::White : CRGB::Black;
    else
        *LED_RAILCOM = CRGB::Black;
}

// ---------------------------------------------------------------------------
//  LED 3 : télémétrie
// ---------------------------------------------------------------------------
void EXCC_Booster_WS2812::updateLedTelemetry()
{
    if (m_faultThermal)
    {
        *LED_TELEM = CRGB::Red;
        return;
    }

    if (m_voltage_mV < EXCC_BOOSTER_MIN_TENSION_mV)
    {
        *LED_TELEM = CRGB::Blue;
        return;
    }

    if (m_current_mA > (EXCC_BOOSTER_MAX_COURANT_mA * 8) / 10)
    {
        *LED_TELEM = CRGB::Orange;
        return;
    }

    *LED_TELEM = CRGB::Green;
}

// ---------------------------------------------------------------------------
//  Accesseurs
// ---------------------------------------------------------------------------
uint16_t EXCC_Booster_WS2812::readCurrent_mA() const { return m_current_mA; }
uint16_t EXCC_Booster_WS2812::readVoltage_mV() const { return m_voltage_mV; }
bool EXCC_Booster_WS2812::isThermalFault() const { return m_faultThermal; }
