#include "EXCC_StatusLed.h"
#include "EXCC_CanBooster.h"
#include "EXCC_Booster_WS2812.h"
#include "CanDccBooster.h"
#include "EXCC_Config.h"
#include <Arduino.h>

extern EXCC_Booster_WS2812 booster;

CRGB *EXCC_StatusLed::LED_CAN     = nullptr;
CRGB *EXCC_StatusLed::LED_RAILCOM = nullptr;
CRGB *EXCC_StatusLed::LED_TELEM   = nullptr;

void EXCC_StatusLed::begin(CRGB *strip)
{
    LED_CAN     = &strip[4];
    LED_RAILCOM = &strip[2];
    LED_TELEM   = &strip[3];
}

void EXCC_StatusLed::update()
{
    updateLedCan();
    updateLedRailcom();
    updateLedTelemetry();
}

void EXCC_StatusLed::updateLedCan()
{
    if (!LED_CAN)
        return;

    uint32_t now  = millis();
    uint32_t last = EXCC_CanBooster::lastCanRxMs();

    if (now - last < 200)
        *LED_CAN = CRGB::Green;
    else if (now - last < 1000)
        *LED_CAN = CRGB::Orange;
    else
        *LED_CAN = CRGB::Red;
}

void EXCC_StatusLed::updateLedRailcom()
{
    if (!LED_RAILCOM)
        return;

    const BoosterTelemetry &t = booster.getTelemetry();

    uint32_t now       = millis();
    uint32_t lastCutout = EXCC_CanBooster::lastCutoutMs();
    bool cutoutActive  = EXCC_CanBooster::isCutoutActive();
    bool cutoutOk      = (now - lastCutout < 50);

    if (t.railcomAddress != BoosterConstants::RAILCOM_NO_ADDRESS)
    {
        *LED_RAILCOM = (millis() & 100) ? CRGB(150, 0, 255) : CRGB::Black;
        return;
    }

    if (cutoutActive)
    {
        *LED_RAILCOM = CRGB::White;
        return;
    }

    if (cutoutOk)
    {
        *LED_RAILCOM = CRGB(0, 0, 20);
        return;
    }

    *LED_RAILCOM = CRGB::Red;
}

void EXCC_StatusLed::updateLedTelemetry()
{
    if (!LED_TELEM)
        return;

    const BoosterTelemetry &t = booster.getTelemetry();

    if (t.error == BoosterError::HARDWARE_FAULT)
    {
        *LED_TELEM = CRGB::Red;
        return;
    }

    if (t.voltage_mV < EXCC_BOOSTER_MIN_TENSION_mV)
    {
        *LED_TELEM = CRGB::Blue;
        return;
    }

    if (t.current_mA > (EXCC_BOOSTER_MAX_COURANT_mA * 8) / 10)
    {
        *LED_TELEM = CRGB::Orange;
        return;
    }

    *LED_TELEM = CRGB::Green;
}
