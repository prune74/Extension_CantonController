/*
 * Module : EXCC_Main (WS2812 Version)
 * Rôle   : Point d’entrée du firmware EXCC (Gestion Canton 2026).
 */

#include "EXCC_Main.h"
#include "EXCC_System.h"
#include "EXCC_Runtime.h"
#include "EXCC_Pins.h"
#include "EXCC_Config.h"

#include "EXCC_Signaux_WS2812.h"
#include "EXCC_LedDirection_WS2812.h"
#include "EXCC_Canton_WS2812.h"

#include "EXCC_Quadrature.h"
#include "EXCC_Compteur.h"
#include "EXCC_Booster_WS2812.h"   // Booster
#include "EXCC_StatusLed.h"        // Nouveau : gestion LEDs Status

#include <FastLED.h>
#include <Arduino.h>

// ============================================================
// Définition des STRIPS WS2812
// ============================================================

// --- Mât H ---
CRGB stripH_feux[9];
CRGB stripH_oeil[1];

// --- Mât AH ---
CRGB stripAH_feux[9];
CRGB stripAH_oeil[1];

// --- Direction H ---
CRGB stripDIR_H[4];

// --- Direction AH ---
CRGB stripDIR_AH[4];

// --- Status ---
// Strip 5 LEDs :
//   LED 0 : Canton
//   LED 1 : Booster état général
//   LED 2 : Booster RailCom
//   LED 3 : Booster télémétrie
//   LED 4 : CAN (activité / erreur)
CRGB g_wsStrip[5];

// ============================================================
// PCA9685 global (servos uniquement)
// ============================================================
Adafruit_PWMServoDriver g_pca9685 = Adafruit_PWMServoDriver();

// ============================================================
// Instances globales EXCC
// ============================================================

// Signaux SNCF (H et AH)
EXCC_Signaux_WS2812 signauxH(stripH_feux, 9, stripH_oeil, 1, ExccSignalLayout::NORMAL);
EXCC_Signaux_WS2812 signauxAH(stripAH_feux, 9, stripAH_oeil, 1, ExccSignalLayout::NORMAL);

// Direction H et AH (4 LED cumulatives chacune)
EXCC_LedDirection_WS2812 directionH_WS(stripDIR_H, 0, 1, 2, 3);
EXCC_LedDirection_WS2812 directionAH_WS(stripDIR_AH, 0, 1, 2, 3);

// Canton WS2812 → LED 0 du strip Booster
EXCC_Canton_WS2812 cantonWS(&g_wsStrip[0]);

// Quadrature
EXCC_Quadrature quadH;
EXCC_Quadrature quadAH;

// ============================================================
// Booster WS2812 — INSTANCE UNIQUE
// ============================================================
EXCC_Booster_WS2812 booster;

// ============================================================
// Cycle de vie EXCC
// ============================================================

void EXCC_Main::begin() noexcept
{
    // ------------------------------------------------------------
    // Initialisation système (GPIO, PCA9685 pour servos, etc.)
    // ------------------------------------------------------------
    EXCC_System::init();

    // ------------------------------------------------------------
    // Initialisation WS2812 — 5 chaînes indépendantes
    // ------------------------------------------------------------

    // Mât H
    FastLED.addLeds<WS2812B, PIN_WS2812_H, GRB>(stripH_feux, 9);
    FastLED.addLeds<WS2812B, PIN_WS2812_H_OEIL, GRB>(stripH_oeil, 1);

    // Mât AH
    FastLED.addLeds<WS2812B, PIN_WS2812_AH, GRB>(stripAH_feux, 9);
    FastLED.addLeds<WS2812B, PIN_WS2812_AH_OEIL, GRB>(stripAH_oeil, 1);

    // Direction H
    FastLED.addLeds<WS2812B, PIN_WS2812_DIR_H, GRB>(stripDIR_H, 4);

    // Direction AH
    FastLED.addLeds<WS2812B, PIN_WS2812_DIR_AH, GRB>(stripDIR_AH, 4);

    // Status (5 LEDs)
    FastLED.addLeds<WS2812B, PIN_WS2812_STATUS, GRB>(g_wsStrip, 5);

    // Efface toutes les LED au démarrage
    FastLED.clear(true);

    // ------------------------------------------------------------
    // Canton WS2812
    // ------------------------------------------------------------
    cantonWS.begin();

    // ------------------------------------------------------------
    // Quadrature H / AH
    // ------------------------------------------------------------
    quadH.begin(PIN_QUAD_H_A, PIN_QUAD_H_B);
    quadAH.begin(PIN_QUAD_AH_A, PIN_QUAD_AH_B);

    quadH.installerInterruptions();
    quadAH.installerInterruptions();

    // ------------------------------------------------------------
    // Compteur essieux
    // ------------------------------------------------------------
    EXCC_Compteur::begin();

    // ------------------------------------------------------------
    // Booster WS2812
    // ------------------------------------------------------------
    booster.begin();

    // ------------------------------------------------------------
    // Status LEDs (CAN pour l’instant)
    // ------------------------------------------------------------
    EXCC_StatusLed::begin(g_wsStrip);
}

void EXCC_Main::loop() noexcept
{
    // Boucle principale EXCC
    EXCC_Runtime::update();

    // Mise à jour des LEDs Status (LED 4 CAN pour l’instant)
    EXCC_StatusLed::update();
}
