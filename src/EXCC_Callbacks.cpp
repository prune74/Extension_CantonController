/*
 * ============================================================================
 *  Module : EXCC_Callbacks (WS2812 Version)
 *  Rôle   : Point d’entrée des commandes CC → EXCC.
 *
 *  Cette version pilote :
 *    - Signaux SNCF WS2812 (H et AH)
 *    - Feux directionnels WS2812 (H et AH)
 *    - Canton WS2812
 *    - Servos via PCA9685
 *    - Aiguilles (anti-blocage)
 *    - Booster / Calibration
 *
 *  Notes :
 *    - Aucun aspect SNCF n’est interprété ici : on ne fait que dispatcher.
 *    - Aucun layout n’est défini ici : EXCC_Signaux_WS2812 s’en charge.
 *    - Aucun protocole n’est décodé ici : tout arrive déjà structuré.
 * ============================================================================
 */

#include "EXCC_Callbacks.h"

#include "EXCC_Signaux_WS2812.h"
#include "EXCC_LedDirection_WS2812.h"
#include "EXCC_Canton_WS2812.h"

#include "EXCC_Servo.h"
#include "EXCC_Switches.h"
#include "EXCC_Calibration.h"
#include "EXCC_Occupation.h"
#include "EXCC_BoosterCore.h"
#include "EXCC_Tension_Booster.h"

#include "EXCC_CAN_CC.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

/* ============================================================================
 *  Instances globales (déclarées dans EXCC_Main.cpp)
 * ============================================================================
 */
extern EXCC_Signaux_WS2812 signauxH;
extern EXCC_Signaux_WS2812 signauxAH;

extern EXCC_LedDirection_WS2812 directionH_WS;
extern EXCC_LedDirection_WS2812 directionAH_WS;

extern EXCC_Canton_WS2812 cantonWS;

/* ============================================================================
 *  ID du canton (reçu via CANTON_ID = 0xFD)
 * ============================================================================
 */
uint8_t g_idCC = 0;

/* ============================================================================
 *  Fonction interne : mapping type SNCF → layout WS2812
 * ============================================================================
 */
static ExccSignalLayout mapTypeToLayout(uint8_t t)
{
    switch (t)
    {
    case SIG_MANOEUVRE:
        return ExccSignalLayout::MANOEUVRE;

    default:
        return ExccSignalLayout::NORMAL;
    }
}

/* ============================================================================
 *  Réception de l’ID du canton (FD)
 * ============================================================================
 */
void EXCC_Callbacks::onCantonID(const uint8_t *data, uint8_t len) noexcept
{
#if EXCC_DEBUG
    Serial.printf("[EXCC] CANTON_ID reçu : %u\n", data[0]);
#endif

    if (len < 1)
        return;

    g_idCC = data[0];
}

/* ============================================================================
 *  Réception PING (FC)
 * ============================================================================
 */
void EXCC_Callbacks::onPing() noexcept
{
#if EXCC_DEBUG
    Serial.println("[EXCC] PING reçu → envoi PONG");
#endif

    EXCC_CAN_CC::sendPong(g_idCC);
}

/* ============================================================================
 *  Configuration signaux (E5)
 * ============================================================================
 */
void EXCC_Callbacks::onConfigSignaux(const uint8_t *data, uint8_t len) noexcept
{
#if EXCC_DEBUG
    Serial.println("[EXCC] --- Configuration signaux (E5) reçue ---");
#endif

    if (len != 4)
        return;

    uint8_t typeH = data[0];
    uint8_t typeAH = data[1];
    uint8_t posH = data[2];
    uint8_t posAH = data[3];

    // 1) Type SNCF
    signauxH.setType(typeH);
    signauxAH.setType(typeAH);

    // 2) Layout WS2812
    signauxH.setLayout(mapTypeToLayout(typeH));
    signauxAH.setLayout(mapTypeToLayout(typeAH));

    // 3) Position physique
    signauxH.setPosition(posH);
    signauxAH.setPosition(posAH);

    // 4) Réappliquer l’aspect courant
    signauxH.setAspect(signauxH.getAspect());
    signauxAH.setAspect(signauxAH.getAspect());

#if EXCC_DEBUG
    Serial.println("[EXCC] --- Fin configuration signaux ---");
#endif
}

/* ============================================================================
 *  Aspects SNCF (E6 / E7)
 * ============================================================================
 */
void EXCC_Callbacks::onAspectHoraire(uint8_t aspect) noexcept
{
    signauxH.setAspect((ExccAspect)aspect);
}

void EXCC_Callbacks::onAspectAntiHoraire(uint8_t aspect) noexcept
{
    signauxAH.setAspect((ExccAspect)aspect);
}

/* ============================================================================
 *  Directions (E8 / E9)
 * ============================================================================
 */
void EXCC_Callbacks::onDirectionHoraire(uint8_t code) noexcept
{
    directionH_WS.setDirection(code);
}

void EXCC_Callbacks::onDirectionAntiHoraire(uint8_t code) noexcept
{
    directionAH_WS.setDirection(code);
}

/* ============================================================================
 *  Occupation voisins (EA)
 * ============================================================================
 */
void EXCC_Callbacks::onOccupationVoisins(uint8_t value) noexcept
{
    EXCC_Occupation::majVoisins(value);
}

/* ============================================================================
 *  Servos (F0 / F1 / F2)
 * ============================================================================
 */
void EXCC_Callbacks::onServoMove(uint8_t servoIndex, uint8_t dir) noexcept
{
    EXCC_Servo::move(servoIndex, dir);
    EXCC_Switches::notifierMouvementDemarre(servoIndex);
}

void EXCC_Callbacks::onServoConfig(uint8_t servoIndex,
                                   uint16_t posDroit,
                                   uint16_t posDevie,
                                   uint16_t speed) noexcept
{
    EXCC_Servo::configure(servoIndex, posDroit, posDevie, speed);
}

void EXCC_Callbacks::onServoTest(uint8_t servoIndex) noexcept
{
    EXCC_Servo::test(servoIndex);
}

/* ============================================================================
 *  Recalibration Booster (F3)
 * ============================================================================
 */
void EXCC_Callbacks::onRecalibrationBooster() noexcept
{
#if EXCC_DEBUG
    Serial.println("[EXCC] Recalibration Booster demandée");
#endif

    EXCC_Calibration::start();
}

/* ============================================================================
 *  Application des seuils (F4)
 * ============================================================================
 */
void EXCC_Callbacks::onSetSeuils(uint16_t libre, uint16_t occupe) noexcept
{
#if EXCC_DEBUG
    Serial.printf("[EXCC] SET SEUILS : libre=%u occupe=%u\n", libre, occupe);
#endif

    EXCC_Calibration::setSeuils(libre, occupe);
}

/* ============================================================================
 *  Booster ON/OFF (F5)
 * ============================================================================
 */
void EXCC_Callbacks::onBoosterPower(uint8_t power) noexcept
{
#if EXCC_DEBUG
    Serial.printf("[EXCC] BOOSTER POWER : %s\n", power ? "ON" : "OFF");
#endif

    EXCC_BoosterCore::setEnabled(power != 0);
}

/* ============================================================================
 *  Profil de voie 12V ou 15V
 * ============================================================================
 */
void EXCC_Callbacks::onProfileVoie(uint8_t profile) noexcept
{
#if EXCC_DEBUG
    Serial.printf("[EXCC] PROFILE_VOIE reçu : %u\n", profile);
#endif

    // profile = 0 → voie N (12V)
    // profile = 1 → voie HO (15V)

    EXCC_Tension_Booster::setTrackProfile(profile == 1);
}
