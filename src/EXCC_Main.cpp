/*
 * Module : EXCC_Main (WS2812 Version)
 * Rôle   : Point d'entrée du firmware EXCC (Chemin de Fer du Chablais (CFC)).
 */

#include "EXCC_Main.h"
#include "EXCC_System.h"
#include "EXCC_Runtime.h"
#include "EXCC_Pins.h"
#include "EXCC_Config.h"

#include "EXCC_Signaux_WS2812.h"
#include "EXCC_LedDirection_WS2812.h"
#include "EXCC_Canton_WS2812.h"

#include "EXCC_Servo.h"
#include "EXCC_Switches.h"
#include "EXCC_Quadrature.h"
#include "EXCC_Compteur.h"
#include "EXCC_Occupation.h"
#include "EXCC_Calibration.h"

#include "EXCC_Booster_WS2812.h"
#include "EXCC_BoosterCore.h"
#include "EXCC_StatusLed.h"

#include "Protocol.h"

#include <Adafruit_PWMServoDriver.h>
#include <FastLED.h>
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

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
// Strip 6 LEDs :
//   LED 0 : Canton
//   LED 1 : Booster état général
//   LED 2 : Booster RailCom
//   LED 3 : Booster télémétrie
//   LED 4 : CAN (activité / erreur)
//   LED 5 : Aiguilles (switches)
//
// PHASE 0 — Correction : 6 LEDs (l'ancien [5] débordait avec addLeds(..., 6)
// et LED_SWITCHES = &strip[5] : corruption mémoire adjacente).
CRGB g_wsStrip[6];

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
    // 1) Init système : I2C + PCA9685 (Wire.begin + g_pca9685.begin)
    //    ⚠️ AVANT tout le reste (fix crash LoadProhibited)
    // ------------------------------------------------------------
    EXCC_System::init();

    // ------------------------------------------------------------
    // 2) WS2812 — 7 chaînes indépendantes
    //    (RMT canaux 0..6 ; le canal 7 reste libre pour le futur DCC)
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

    // Status (6 LEDs) — PHASE 0 : 6, cohérent avec CRGB g_wsStrip[6]
    FastLED.addLeds<WS2812B, PIN_WS2812_STATUS, GRB>(g_wsStrip, 6);

    FastLED.setBrightness(EXCC_WS2812_GLOBAL_BRIGHTNESS);

    // Efface toutes les LED au démarrage
    FastLED.clear(true);

    // ------------------------------------------------------------
    // 3) Modules — chaque begin() exactement UNE fois
    // ------------------------------------------------------------

    // Servos (PCA9685) — maintenant que le bus I2C est ouvert
    EXCC_Servo::begin();

    // Micro-switchs d'aiguilles
    EXCC_Switches::begin();

    // Canton WS2812 : état initial = LIBRE
    cantonWS.begin();

    // Signaux SNCF WS2812 : aspect initial = MASQUÉ
    signauxH.setAspect(ExccAspect::ASPECT_MASQUE);
    signauxAH.setAspect(ExccAspect::ASPECT_MASQUE);

    // Feux directionnels WS2812 : état initial = OFF
    directionH_WS.setDirection(0);
    directionAH_WS.setDirection(0);

    // Premier rendu — APRÈS les addLeds (l'ancien ordre affichait
    // avant l'existence des contrôleurs)
    FastLED.show();

    // Quadrature H / AH
    quadH.begin(PIN_QUAD_H_A, PIN_QUAD_H_B);
    quadAH.begin(PIN_QUAD_AH_A, PIN_QUAD_AH_B);
    quadH.installerInterruptions();
    quadAH.installerInterruptions();

    // Compteur essieux
    EXCC_Compteur::begin();

    // Occupation
    EXCC_Occupation::begin();

    // Booster WS2812 (DRV8874 + ADC) — une seule fois ici
    booster.begin();

    // Status LEDs (pointeurs sur g_wsStrip, LED 0..5)
    EXCC_StatusLed::begin(g_wsStrip);

    // ------------------------------------------------------------
    // 4) Tâche booster — EN DERNIER : tout le matériel est prêt.
    //    (l'ancien code la lançait au tout début → course au boot)
    // ------------------------------------------------------------
    EXCC_BoosterCore::startTask();
}

void EXCC_Main::loop() noexcept
{
    // Boucle principale EXCC
    EXCC_Runtime::update();

    // Mise à jour des LEDs Status (LED 4 CAN pour l'instant)
    EXCC_StatusLed::update();
}