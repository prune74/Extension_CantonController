#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <FastLED.h>
#include <stdint.h>
#include "EXCC_Config.h"

class EXCC_LedDirection_WS2812
{
public:
    EXCC_LedDirection_WS2812(CRGB *ledArray,
                             uint8_t idx0,
                             uint8_t idx1,
                             uint8_t idx2,
                             uint8_t idx3) noexcept;

    bool setDirection(uint8_t direction) noexcept;
    void setIntensity(uint8_t index, uint8_t intensite) noexcept;

private:
    CRGB *leds;
    uint8_t idx[4];
    uint8_t intensites[4];
    bool etats[4];
};
