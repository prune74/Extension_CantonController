#include "EXCC_StatusLed.h"
#include "EXCC_CanBooster.h"
#include <Arduino.h>

// LED CAN = LED 4 du strip Status
CRGB *EXCC_StatusLed::LED_CAN = nullptr;

void EXCC_StatusLed::begin(CRGB *strip)
{
    LED_CAN = &strip[4];   // LED CAN
}

void EXCC_StatusLed::update()
{
    updateLedCan();
}

void EXCC_StatusLed::updateLedCan()
{
    uint32_t now  = millis();
    uint32_t last = EXCC_CanBooster::lastCanRxMs();

    if (now - last < 200)
    {
        // CAN actif
        *LED_CAN = CRGB::Green;
    }
    else if (now - last < 1000)
    {
        // CAN silencieux
        *LED_CAN = CRGB::Orange;
    }
    else
    {
        // CAN KO
        *LED_CAN = CRGB::Red;
    }
}
