/*
 * Module : EXCC_Runtime
 * Rôle   : Boucle d’exécution principale EXCC et passerelles CAN/UART.
 */

#include "EXCC_Runtime.h"

#include "EXCC_Signaux_WS2812.h"
#include "EXCC_Ponctuel.h"
#include "EXCC_Occupation.h"
#include "EXCC_Compteur.h"
#include "EXCC_BoosterUart.h"
#include "EXCC_CanCommands.h"
#include "EXCC_Calibration.h"
#include "EXCC_CAN_CC.h"
#include "EXCC_Canton_WS2812.h"
#include "EXCC_Switches.h"
#include "EXCC_Config.h"

#include "Ws2812All.h"
#include <Arduino.h>

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

extern EXCC_Signaux_WS2812 signauxH;
extern EXCC_Signaux_WS2812 signauxAH;
extern EXCC_Canton_WS2812 cantonWS;
extern EXCC_BoosterUart boosterUart;
extern uint8_t g_idCC;

namespace
{
bool s_calibrationEnvoyee = false;
uint32_t s_lastBoosterReportMs = 0;
}

void EXCC_Runtime::update() noexcept
{
    const uint32_t now = millis();

    boosterUart.process();
    EXCC_CanCommands::process();

    EXCC_Occupation::majCourant(
        boosterUart.current_mA(),
        boosterUart.isCurrentValid());
    EXCC_Occupation::majCompteur(EXCC_Compteur::compteurGlobal());
    EXCC_Occupation::majPonctuels(
        EXCC_Ponctuel::estActifH(),
        EXCC_Ponctuel::estActifAH());
    EXCC_Occupation::calculerOccupationFinale();

    if (EXCC_Calibration::isRunning())
        s_calibrationEnvoyee = false;
    else if (EXCC_Calibration::isDone() && !s_calibrationEnvoyee)
    {
        s_calibrationEnvoyee = true;
        EXCC_CAN_CC::envoyerCalibBooster(
            g_idCC,
            EXCC_Calibration::getSeuilLibre(),
            EXCC_Calibration::getSeuilOccupe());
    }

    if (now - s_lastBoosterReportMs >= 200)
    {
        s_lastBoosterReportMs = now;
        ExccBoosterEtat etat = BOOSTER_ERREUR;

        if (boosterUart.isSampleFresh())
        {
            const uint16_t courant = boosterUart.current_mA();
            const uint16_t tension = boosterUart.voltage_mV();
            switch (boosterUart.state())
            {
            case ExccRemoteBoosterState::FAULT:
                etat = (courant > EXCC_BOOSTER_MAX_COURANT_mA)
                           ? BOOSTER_COURT_CIRCUIT
                           : (tension < EXCC_BOOSTER_MIN_TENSION_mV
                                  ? BOOSTER_SOUS_TENSION
                                  : BOOSTER_ERREUR);
                break;
            case ExccRemoteBoosterState::OVERHEAT:
                etat = BOOSTER_SURCHAUFFE;
                break;
            case ExccRemoteBoosterState::OFF:
                etat = boosterUart.powerRequested() ? BOOSTER_ERREUR : BOOSTER_OFF;
                break;
            case ExccRemoteBoosterState::RETRY:
                etat = BOOSTER_ERREUR;
                break;
            case ExccRemoteBoosterState::OK:
                if (!boosterUart.powerRequested())
                    etat = BOOSTER_OFF;
                else if (tension < EXCC_BOOSTER_MIN_TENSION_mV)
                    etat = BOOSTER_SOUS_TENSION;
                else if (courant > EXCC_BOOSTER_MAX_COURANT_mA)
                    etat = BOOSTER_COURT_CIRCUIT;
                else
                    etat = BOOSTER_OK;
                break;
            }
        }

        EXCC_CAN_CC::envoyerBooster(
            g_idCC,
            etat,
            min<uint16_t>(boosterUart.current_mA() / 10, 255),
            min<uint16_t>(boosterUart.voltage_mV() / 100, 255));
    }

    uint16_t railcomAddress = 0;
    uint8_t railcomType = 0;
    while (boosterUart.takeRailComEvent(railcomAddress, railcomType))
        EXCC_CAN_CC::envoyerRailcom(g_idCC, railcomType, railcomAddress);

    signauxH.update(now);
    signauxAH.update(now);
    cantonWS.update();
    EXCC_Switches::update();
    Ws2812All::showAll();
}
