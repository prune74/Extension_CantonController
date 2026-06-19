/*
 * ============================================================================
 *  Module : EXCC_System (WS2812 Version)
 *  Rôle   : Initialisation complète du système EXCC (Gestion Canton 2026).
 *
 *  Fonctionnement général :
 *    - Configure l’UART RS485 (réception CC → EXCC)
 *    - Initialise tous les modules EXCC :
 *         • Servos (PCA9685)
 *         • Canton WS2812 (occupation + mouvement + erreur)
 *         • Micro‑switchs d’aiguilles
 *         • Signaux SNCF WS2812 (H et AH)
 *         • Feux directionnels WS2812 (H et AH)
 *         • Booster (tâche FreeRTOS)
 *
 *  Notes Gestion Canton 2026 :
 *    - Le MCP23017 n’est plus utilisé.
 *    - Le sens H/AH n’est plus déterminé par un DIP.
 *    - Les feux directionnels ne passent plus par le PCA9685.
 *    - Le canton ne passe plus par le PCA9685.
 *    - Le booster reste inchangé.
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

/* ============================================================================
 * init()
 * ----------------------------------------------------------------------------
 * Point d’entrée de l’initialisation EXCC.
 * Appelé une seule fois au démarrage.
 * ============================================================================
 */
void EXCC_System::init() noexcept
{
    // --------------------------------------------------------
    // 1) UART CC → EXCC (réception RS485)
    // --------------------------------------------------------
    EXCC_UartRx::begin(Serial1, EXCC_UART_BAUDRATE);

    // --------------------------------------------------------
    // 2) PCA9685 (servos uniquement)
    // --------------------------------------------------------
    EXCC_Servo::begin();

    // --------------------------------------------------------
    // 3) Canton WS2812 : état initial = LIBRE (après animation)
    // --------------------------------------------------------
    cantonWS.begin();

    // --------------------------------------------------------
    // 4) Micro‑switchs d’aiguilles
    // --------------------------------------------------------
    EXCC_Switches::begin();

    // --------------------------------------------------------
    // 5) Signaux SNCF WS2812 : aspect initial = MASQUÉ
    // --------------------------------------------------------
    signauxH.setAspect(ASPECT_MASQUE);
    signauxAH.setAspect(ASPECT_MASQUE);

    // --------------------------------------------------------
    // 6) Feux directionnels WS2812 : état initial = OFF
    // --------------------------------------------------------
    directionH_WS.setDirection(0);
    directionAH_WS.setDirection(0);

    FastLED.show(); // applique l’état initial

    // --------------------------------------------------------
    // 7) Calibration Booster — initialisation des seuils
    // --------------------------------------------------------
    EXCC_Calibration::init();

    // --------------------------------------------------------
    // 8) Booster (tâche FreeRTOS)
    // --------------------------------------------------------
    EXCC_BoosterCore::startTask();
}
