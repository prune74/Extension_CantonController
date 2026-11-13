#include "EXCC_StatusLed.h"
#include "EXCC_CanCommands.h"
#include "EXCC_BoosterUart.h"
#include "Protocol.h"
#include "EXCC_Switches.h"
#include "EXCC_Config.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

extern EXCC_BoosterUart boosterUart;

// Pointeurs vers les LEDs du strip
WsCrgb *EXCC_StatusLed::LED_CAN = nullptr;
WsCrgb *EXCC_StatusLed::LED_RAILCOM = nullptr;
WsCrgb *EXCC_StatusLed::LED_TELEM = nullptr;
WsCrgb *EXCC_StatusLed::LED_STATE = nullptr;
WsCrgb *EXCC_StatusLed::LED_SWITCHES = nullptr;

void EXCC_StatusLed::begin(WsCrgb *strip)
{
    LED_CAN = &strip[4];
    LED_RAILCOM = &strip[2];
    LED_TELEM = &strip[3];
    LED_STATE = &strip[1];
    LED_SWITCHES = &strip[5];
}

void EXCC_StatusLed::update()
{
    updateLedCan();
    updateLedRailcom();
    updateLedTelemetry();
    updateLedState();
    updateLedSwitches();
}

/* ============================================================
 * LED CAN
 * ============================================================ */
void EXCC_StatusLed::updateLedCan()
{
    if (!LED_CAN)
        return;

    uint32_t now = millis();
    uint32_t last = EXCC_CanCommands::lastRxMs();

    if (last != 0 && now - last < 200)
        *LED_CAN = WsCrgb::Green();
    else if (last != 0 && now - last < 1000)
        *LED_CAN = WsCrgb::Orange();
    else
        *LED_CAN = WsCrgb::Red();
}

/* ============================================================
 * LED RailCom
 * ============================================================ */
void EXCC_StatusLed::updateLedRailcom()
{
    if (!LED_RAILCOM)
        return;

    if (!boosterUart.isSampleFresh())
    {
        *LED_RAILCOM = WsCrgb::Red();
        return;
    }

    if (boosterUart.hasRecentRailCom())
    {
        *LED_RAILCOM = (millis() & 100) ? WsCrgb(150, 0, 255) : WsCrgb::Black();
        return;
    }

    *LED_RAILCOM = WsCrgb(0, 0, 20);
}

/* ============================================================
 * LED Télémétrie
 * ============================================================ */
void EXCC_StatusLed::updateLedTelemetry()
{
    if (!LED_TELEM)
        return;

    const uint16_t courant = boosterUart.current_mA();
    const uint16_t tension = boosterUart.voltage_mV();

    if (!boosterUart.isSampleFresh() ||
        boosterUart.state() == ExccRemoteBoosterState::FAULT ||
        boosterUart.state() == ExccRemoteBoosterState::OVERHEAT)
    {
        *LED_TELEM = WsCrgb::Red();
        return;
    }

    if (tension < EXCC_BOOSTER_MIN_TENSION_mV)
    {
        *LED_TELEM = WsCrgb::Blue();
        return;
    }

    if (courant > (EXCC_BOOSTER_MAX_COURANT_mA * 8) / 10)
    {
        *LED_TELEM = WsCrgb::Orange();
        return;
    }

    *LED_TELEM = WsCrgb::Green();
}

/* ============================================================
 * LED État général Booster
 * ============================================================ */
void EXCC_StatusLed::updateLedState()
{
    if (!LED_STATE)
        return;

    ExccBoosterEtat etat;

    if (!boosterUart.isSampleFresh())
        etat = BOOSTER_ERREUR;
    else if (!boosterUart.powerRequested())
        etat = BOOSTER_OFF;
    else if (boosterUart.state() == ExccRemoteBoosterState::OFF)
        etat = BOOSTER_ERREUR;
    else if (boosterUart.state() == ExccRemoteBoosterState::OVERHEAT)
        etat = BOOSTER_SURCHAUFFE;
    else if (boosterUart.state() == ExccRemoteBoosterState::FAULT)
        etat = BOOSTER_ERREUR;
    else if (boosterUart.voltage_mV() < EXCC_BOOSTER_MIN_TENSION_mV)
        etat = BOOSTER_SOUS_TENSION;
    else if (boosterUart.current_mA() > EXCC_BOOSTER_MAX_COURANT_mA)
        etat = BOOSTER_COURT_CIRCUIT;
    else
        etat = BOOSTER_OK;

    switch (etat)
    {
    case BOOSTER_OFF:
        *LED_STATE = WsCrgb::Red();
        break;

    case BOOSTER_OK:
        *LED_STATE = WsCrgb::Green();
        break;

    case BOOSTER_COURT_CIRCUIT:
        *LED_STATE = (millis() & 200) ? WsCrgb::Red() : WsCrgb::Black();
        break;

    case BOOSTER_SOUS_TENSION:
        *LED_STATE = WsCrgb::Blue();
        break;

    case BOOSTER_SURCHAUFFE:
    {
        uint8_t p = ws_sin8(millis() >> 3);
        *LED_STATE = WsCrgb(p, 0, p);
        break;
    }

    case BOOSTER_ERREUR:
        *LED_STATE = (millis() & 300) ? WsCrgb::Red() : WsCrgb::Purple();
        break;
    }
}

/* ============================================================
 * LED Switches — Diagnostic aiguilles (Variante 4D répétitive)
 * ============================================================ */
void EXCC_StatusLed::updateLedSwitches()
{
    if (!LED_SWITCHES)
        return;

    static uint8_t flashCount = 0;
    static uint8_t flashesToDo = 0;
    static uint32_t timer = 0;
    static bool ledOn = false;

    uint32_t now = millis();

    uint8_t worst = EXCC_Switches::getWorstState();
    bool moving = EXCC_Switches::anyMovement();

    // OK
    if (worst == static_cast<uint8_t>(ExccCode::ETAT_OK) && !moving)
    {
        *LED_SWITCHES = WsCrgb::Green();
        flashCount = 0;
        flashesToDo = 0;
        return;
    }

    // Mouvement
    if (worst == static_cast<uint8_t>(ExccCode::ETAT_OK) && moving)
    {
        *LED_SWITCHES = WsCrgb::Orange();
        flashCount = 0;
        flashesToDo = 0;
        return;
    }

    // Erreurs
    if (worst == static_cast<uint8_t>(ExccCode::ETAT_ERREUR))
        flashesToDo = 1; // INDET / INCOHÉRENT
    else if (worst == static_cast<uint8_t>(ExccCode::ETAT_BLOQUE))
        flashesToDo = 3; // BLOQUÉ

    const uint16_t FLASH_ON_MS = 150;
    const uint16_t FLASH_OFF_MS = 150;
    const uint16_t PAUSE_MS = 600;

    if (flashCount < flashesToDo)
    {
        if (!ledOn && now - timer >= FLASH_OFF_MS)
        {
            *LED_SWITCHES = WsCrgb::Red();
            ledOn = true;
            timer = now;
        }
        else if (ledOn && now - timer >= FLASH_ON_MS)
        {
            *LED_SWITCHES = WsCrgb::Black();
            ledOn = false;
            timer = now;
            flashCount++;
        }
    }
    else
    {
        if (now - timer >= PAUSE_MS)
        {
            flashCount = 0;
            ledOn = false;
            timer = now;
        }
        else
        {
            *LED_SWITCHES = WsCrgb::Black();
        }
    }
}