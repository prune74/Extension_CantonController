#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include "WsCrgb.h"

/*
 * ============================================================
 *  EXCC_StatusLed.h — Gestion des LEDs de statut Chemin de Fer du Chablais (CFC)
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
    static void begin(WsCrgb *strip);

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
    static WsCrgb *LED_CAN;
    static WsCrgb *LED_RAILCOM;
    static WsCrgb *LED_TELEM;
    static WsCrgb *LED_STATE;
    static WsCrgb *LED_SWITCHES;
};
