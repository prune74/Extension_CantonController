#pragma once
#include <FastLED.h>

class EXCC_StatusLed
{
public:
    // Initialise les pointeurs vers les LEDs
    static void begin(CRGB *strip);

    // Mise à jour périodique (appelée dans la loop ou tâche 1 ms)
    static void update();

private:
    static void updateLedCan();

    static CRGB *LED_CAN;
};
