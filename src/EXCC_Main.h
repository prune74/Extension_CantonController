#pragma once
#include <stdint.h>
#include <Adafruit_PWMServoDriver.h>

#include "EXCC_Signaux_WS2812.h"
#include "EXCC_LedDirection_WS2812.h"
#include "EXCC_Canton_WS2812.h"
#include "EXCC_Quadrature.h"
#include "EXCC_Compteur.h"
#include "EXCC_Booster_WS2812.h"

/*
 * ============================================================
 *  Interface EXCC_Main — Version WS2812 (Gestion Canton 2026)
 * ============================================================
 *
 * Architecture générale :
 *   - Signaux SNCF H / AH (WS2812)
 *   - Feux directionnels H / AH (WS2812/WS2815, 4 LED cumulatives chacun)
 *   - Canton (WS2812)
 *   - Quadrature H / AH
 *   - Compteur essieux
 *   - Booster (DCC + RailCom HF)
 *
 * Le PCA9685 reste utilisé uniquement pour :
 *   - Servos
 *   - Modules nécessitant PWM analogique
 */

namespace EXCC_Main
{
    void begin() noexcept;
    void loop() noexcept;
}

/* ============================================================
   Variables / objets globaux partagés
   ============================================================ */

// PCA9685 global (servos uniquement)
extern Adafruit_PWMServoDriver g_pca9685;

// Signaux SNCF (WS2812)
extern EXCC_Signaux_WS2812 signauxH;   // Mât horaire
extern EXCC_Signaux_WS2812 signauxAH;  // Mât anti-horaire

// Feux directionnels (WS2812 / WS2815)
extern EXCC_LedDirection_WS2812 directionH_WS;   // Direction horaire
extern EXCC_LedDirection_WS2812 directionAH_WS;  // Direction anti-horaire

// Canton (WS2812)
extern EXCC_Canton_WS2812 cantonWS;

// Quadrature (H et AH)
extern EXCC_Quadrature quadH;
extern EXCC_Quadrature quadAH;

// Compteur essieux
// (pas besoin d’extern : tout est statique dans EXCC_Compteur)

// Booster (DCC + RailCom HF)
extern EXCC_Booster_WS2812 booster;
