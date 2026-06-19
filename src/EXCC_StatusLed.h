#pragma once
#include <FastLED.h>

class EXCC_StatusLed
{
public:
    static void begin(CRGB *strip);
    static void update();

private:
    static void updateLedCan();

    static CRGB *LED_CAN;
};
