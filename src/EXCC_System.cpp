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
#include "EXCC_UartRx.h"
#include "EXCC_BoosterCore.h"
#include "EXCC_Calibration.h"
#include "EXCC_Pins.h"
#include "EXCC_Main.h"
#include "EXCC_Config.h"
#include "Exploration_Protocol.h"

#include "EXCC_Booster_WS2812.h"   // <-- NOUVEAU MODULE

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <FastLED.h>

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
    // 1) UART CC → EXCC (réception RS485)
    EXCC_UartRx::begin(Serial1, EXCC_UART_BAUDRATE);

    // 2) PCA9685 (servos uniquement)
    EXCC_Servo::begin();

    // 3) Canton WS2812 : état initial = LIBRE
    cantonWS.begin();

    // 4) Micro‑switchs d’aiguilles
    EXCC_Switches::begin();

    // 5) Signaux SNCF WS2812 : aspect initial = MASQUÉ
    signauxH.setAspect(ASPECT_MASQUE);
    signauxAH.setAspect(ASPECT_MASQUE);

    // 6) Feux directionnels WS2812 : état initial = OFF
    directionH_WS.setDirection(0);
    directionAH_WS.setDirection(0);

    FastLED.show();

    // 7) Calibration Booster — seuils par défaut
    EXCC_Calibration::init();

    // 8) Booster — initialisation du module
    booster.begin();   // <-- AJOUT ESSENTIEL

    // 9) Booster — tâche FreeRTOS
    EXCC_BoosterCore::startTask();
}
