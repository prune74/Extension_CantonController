/*
 * ============================================================================
 *  Module : EXCC_System (WS2812 Version)
 * ============================================================================
 */

#include "EXCC_System.h"

#include "EXCC_Servo.h"
#include "EXCC_Switches.h"
#include "EXCC_Canton_WS2812.h"
#include "EXCC_Signaux_WS2812.h"
#include "EXCC_LedDirection_WS2812.h"
#include "EXCC_BoosterCore.h"
#include "EXCC_Calibration.h"
#include "EXCC_Pins.h"
#include "EXCC_Main.h"
#include "EXCC_Config.h"
#include "Protocol.h"

#include "EXCC_Occupation.h"
#include "EXCC_Booster_WS2812.h"

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <FastLED.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

// Instances globales déclarées dans EXCC_Main.cpp
extern Adafruit_PWMServoDriver g_pca9685;

extern EXCC_Signaux_WS2812 signauxH;
extern EXCC_Signaux_WS2812 signauxAH;

extern EXCC_LedDirection_WS2812 directionH_WS;
extern EXCC_LedDirection_WS2812 directionAH_WS;

extern EXCC_Canton_WS2812 cantonWS;

// Booster global
extern EXCC_Booster_WS2812 booster;

/* ============================================================================
 * init()
 * ============================================================================
 */
void EXCC_System::init() noexcept
{
    // PCA9685 (servos uniquement)
    EXCC_Servo::begin();

    // Canton WS2812 : état initial = LIBRE
    cantonWS.begin();

    // Micro‑switchs d’aiguilles
    EXCC_Switches::begin();

    // Signaux SNCF WS2812 : aspect initial = MASQUÉ
    signauxH.setAspect(ExccAspect::ASPECT_MASQUE);
    signauxAH.setAspect(ExccAspect::ASPECT_MASQUE);

    // Feux directionnels WS2812 : état initial = OFF
    directionH_WS.setDirection(0);
    directionAH_WS.setDirection(0);

    FastLED.show();

    // Calibration Booster — seuils par défaut
    EXCC_Calibration::init();

    // Occupation — initialisation du module
    EXCC_Occupation::begin();

    // Booster — initialisation du module
    booster.begin();

    // Booster — tâche FreeRTOS
    EXCC_BoosterCore::startTask();
}
