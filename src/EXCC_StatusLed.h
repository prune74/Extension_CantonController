#pragma once
#include <FastLED.h>

class EXCC_StatusLed
{
public:
    static void begin(CRGB *strip);
    static void update();

private:
    static void updateLedCan();
    static void updateLedRailcom();
    static void updateLedTelemetry();
    static void updateLedState();

    static CRGB *LED_CAN;
    static CRGB *LED_RAILCOM;
    static CRGB *LED_TELEM;
    static CRGB *LED_STATE;
};
