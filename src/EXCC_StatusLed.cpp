#include "EXCC_StatusLed.h"
#include "EXCC_CanBooster.h"
#include <Arduino.h>

CRGB *EXCC_StatusLed::LED_CAN = nullptr;

void EXCC_StatusLed::begin(CRGB *strip)
{
    // LED 4 = CAN
    LED_CAN = &strip[4];
}

void EXCC_StatusLed::update()
{
    updateLedCan();
}

void EXCC_StatusLed::updateLedCan()
{
    if (!LED_CAN)
        return;

    uint32_t now  = millis();
    uint32_t last = EXCC_CanBooster::lastCanRxMs();

    if (now - last < 200)
    {
        *LED_CAN = CRGB::Green;     // CAN actif
    }
    else if (now - last < 1000)
    {
        *LED_CAN = CRGB::Orange;    // CAN silencieux
    }
    else
    {
        *LED_CAN = CRGB::Red;       // CAN KO
    }
}
