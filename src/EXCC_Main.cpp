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
#include <Ws2812Universal.h>
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
WsCrgb stripH_feux[9];
WsCrgb stripH_oeil[1];

// --- Mât AH ---
WsCrgb stripAH_feux[9];
WsCrgb stripAH_oeil[1];

// --- Direction H ---
WsCrgb stripDIR_H[4];

// --- Direction AH ---
WsCrgb stripDIR_AH[4];

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
WsCrgb g_wsStrip[6];

static_assert(sizeof(WsCrgb) == 3, "WsCrgb must be RGB888");

class EXCC_Ws2812Config final : public Ws2812ConfigProvider
{
public:
    EXCC_Ws2812Config()
        : m_buses{
              {true, PIN_WS2812_H, 9, RMT_CHANNEL_0, EXCC_WS2812_GLOBAL_BRIGHTNESS,
               reinterpret_cast<uint8_t *>(stripH_feux)},
              {true, PIN_WS2812_H_OEIL, 1, RMT_CHANNEL_1, EXCC_WS2812_GLOBAL_BRIGHTNESS,
               reinterpret_cast<uint8_t *>(stripH_oeil)},
              {true, PIN_WS2812_AH, 9, RMT_CHANNEL_2, EXCC_WS2812_GLOBAL_BRIGHTNESS,
               reinterpret_cast<uint8_t *>(stripAH_feux)},
              {true, PIN_WS2812_AH_OEIL, 1, RMT_CHANNEL_3, EXCC_WS2812_GLOBAL_BRIGHTNESS,
               reinterpret_cast<uint8_t *>(stripAH_oeil)},
              {true, PIN_WS2812_DIR_H, 4, RMT_CHANNEL_4, EXCC_WS2812_GLOBAL_BRIGHTNESS,
               reinterpret_cast<uint8_t *>(stripDIR_H)},
              {true, PIN_WS2812_DIR_AH, 4, RMT_CHANNEL_5, EXCC_WS2812_GLOBAL_BRIGHTNESS,
               reinterpret_cast<uint8_t *>(stripDIR_AH)},
              {true, PIN_WS2812_STATUS, 6, RMT_CHANNEL_6, EXCC_WS2812_GLOBAL_BRIGHTNESS,
               reinterpret_cast<uint8_t *>(g_wsStrip)}}
    {
    }

    uint8_t busCount() const override { return 7; }

    const Ws2812BusConfig &busConfig(uint8_t index) const override
    {
        return m_buses[index < 7 ? index : 0];
    }

private:
    Ws2812BusConfig m_buses[7];
};

static EXCC_Ws2812Config s_ws2812Config;

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
    // 2) WS2812 — 7 bus RMT indépendants sur les canaux 0..6.
    // ------------------------------------------------------------
    Ws2812Init::begin(s_ws2812Config);
    Ws2812All::setBrightnessAll(EXCC_WS2812_GLOBAL_BRIGHTNESS);
    for (uint8_t i = 0; i < s_ws2812Config.busCount(); ++i)
    {
        if (Ws2812Bus::bus(i).isValid())
            Ws2812Bus::bus(i).clear();
    }

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

    // Premier rendu après initialisation des bus et des états initiaux.
    Ws2812All::showAll();

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