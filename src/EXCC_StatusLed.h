#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <FastLED.h>

/*
 * ============================================================
 *  EXCC_StatusLed.h — Gestion des LEDs de statut Discovery 2026
 * ------------------------------------------------------------
 *  LEDs :
 *    LED 0 : Canton (gérée ailleurs)
 *    LED 1 : État général Booster
 *    LED 2 : RailCom
 *    LED 3 : Télémétrie
 *    LED 4 : CAN
 *    LED 5 : Switches (diagnostic aiguilles — mode 4D répétitif)
 *
 *  Contraintes :
 *    - Aucun malloc
 *    - update() appelé ~1 kHz
 *    - Aucun blocage (pas de delay)
 * ============================================================
 */

class EXCC_StatusLed
{
public:
    // Initialise les pointeurs vers les LEDs du strip
    static void begin(CRGB *strip);

    // Mise à jour de toutes les LEDs
    static void update();

private:
    // LEDs individuelles
    static void updateLedCan();
    static void updateLedRailcom();
    static void updateLedTelemetry();
    static void updateLedState();
    static void updateLedSwitches();

    // Pointeurs vers les LEDs du strip
    static CRGB *LED_CAN;
    static CRGB *LED_RAILCOM;
    static CRGB *LED_TELEM;
    static CRGB *LED_STATE;
    static CRGB *LED_SWITCHES;
};
