#include "EXCC_StatusLed.h"
#include "EXCC_CanBooster.h"
#include "EXCC_Booster_WS2812.h"
#include "CanDccBooster.h"

#include "EXCC_Booster_WS2812.h"
#include <Arduino.h>

// Booster global (pour accéder à la télémétrie RailCom)
extern EXCC_Booster_WS2812 booster;

CRGB *EXCC_StatusLed::LED_CAN = nullptr;
CRGB *EXCC_StatusLed::LED_RAILCOM = nullptr;

void EXCC_StatusLed::begin(CRGB *strip)
{
    // LED 4 = CAN
    LED_CAN = &strip[4];

    // LED 2 = RailCom + Cutout
    LED_RAILCOM = &strip[2];
}

void EXCC_StatusLed::update()
{
    updateLedCan();
    updateLedRailcom();
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

void EXCC_StatusLed::updateLedRailcom()
{
    if (!LED_RAILCOM)
        return;

    const BoosterTelemetry &t = booster.getTelemetry();

    uint32_t now = millis();
    uint32_t lastCutout = EXCC_CanBooster::lastCutoutMs();
    bool cutoutActive = EXCC_CanBooster::isCutoutActive();
    bool cutoutOk = (now - lastCutout < 50);

    // 1) Priorité : adresse RailCom détectée
    if (t.railcomAddress != BoosterConstants::RAILCOM_NO_ADDRESS)
    {
        // Violet clignotant
        *LED_RAILCOM = (millis() & 100) ? CRGB(150, 0, 255) : CRGB::Black;
        return;
    }

    // 2) Cutout actif → blanc
    if (cutoutActive)
    {
        *LED_RAILCOM = CRGB::White;
        return;
    }

    // 3) Cutout OK mais fermé → bleu sombre
    if (cutoutOk)
    {
        *LED_RAILCOM = CRGB(0, 0, 20);
        return;
    }

    // 4) Cutout KO → rouge
    *LED_RAILCOM = CRGB::Red;
}
