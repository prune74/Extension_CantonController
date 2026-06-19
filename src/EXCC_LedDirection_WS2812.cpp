#include "EXCC_LedDirection_WS2812.h"
#include <FastLED.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

EXCC_LedDirection_WS2812::EXCC_LedDirection_WS2812(CRGB *ledArray,
                                                   uint8_t idx0,
                                                   uint8_t idx1,
                                                   uint8_t idx2,
                                                   uint8_t idx3) noexcept
    : leds(ledArray)
{
    idx[0] = idx0;
    idx[1] = idx1;
    idx[2] = idx2;
    idx[3] = idx3;

    // États initiaux
    for (uint8_t i = 0; i < 4; i++)
        etats[i] = false;

    // Intensités configurables (0–255)
    intensites[0] = EXCC_DIR_LED0_INTENSITE;
    intensites[1] = EXCC_DIR_LED1_INTENSITE;
    intensites[2] = EXCC_DIR_LED2_INTENSITE;
    intensites[3] = EXCC_DIR_LED3_INTENSITE;

    // LEDs OFF au démarrage
    for (uint8_t i = 0; i < 4; i++)
        leds[idx[i]] = CRGB::Black;

    FastLED.show();
}

bool EXCC_LedDirection_WS2812::setDirection(uint8_t direction) noexcept
{
    if (direction > 4)
        direction = 4;

    bool changed = false;

    for (uint8_t i = 0; i < 4; i++)
    {
        const bool on = (i < direction);

        if (etats[i] != on)
            changed = true;

        etats[i] = on;

        if (on)
        {
            // Intensité 0–255 → luminosité WS2812
            leds[idx[i]] = CRGB(intensites[i], intensites[i], intensites[i]);
        }
        else
        {
            leds[idx[i]] = CRGB::Black;
        }
    }

    if (changed)
        FastLED.show();

    return changed;
}

void EXCC_LedDirection_WS2812::setIntensity(uint8_t index, uint8_t intensite) noexcept
{
    if (index >= 4)
        return;

    intensites[index] = intensite;

    if (etats[index])
    {
        leds[idx[index]] = CRGB(intensite, intensite, intensite);
        FastLED.show();
    }
}
