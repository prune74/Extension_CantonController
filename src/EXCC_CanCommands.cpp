#include "EXCC_CanCommands.h"
#include "EXCC_Callbacks.h"
#include "Protocol.h"
#include "CanBus.h"
#include "CanMsg.h"
#include "CanInit.h"
#include "EXCC_CanConfig.h"
#include <Arduino.h>

namespace
{
EXCC_CanConfig s_canConfig;
uint32_t s_lastRxMs = 0;
bool s_ccSeen = false;
}

// ---------------------------------------------------------------------------
// Traitement d’une commande CC → EXCC
// ---------------------------------------------------------------------------
static void handleFrame(uint8_t commande, const CanMsg &frame)
{
    const uint8_t *d = frame.data;

    switch (Cmd_CC_to_EXCC(commande))
    {
    case Cmd_CC_to_EXCC::CANTON_ID:
        if (frame.dlc < 1)
            break;
        EXCC_Callbacks::onCantonID(d, frame.dlc);
        break;

    case Cmd_CC_to_EXCC::PING:
        EXCC_Callbacks::onPing();
        break;

    case Cmd_CC_to_EXCC::CONFIG_SIGNAUX:
        if (frame.dlc != 4)
            break;
        EXCC_Callbacks::onConfigSignaux(d, frame.dlc);
        break;

    case Cmd_CC_to_EXCC::ASPECT_HORAIRE:
        if (frame.dlc < 1)
            break;
        EXCC_Callbacks::onAspectHoraire(d[0]);
        break;

    case Cmd_CC_to_EXCC::ASPECT_ANTIHORAIRE:
        if (frame.dlc < 1)
            break;
        EXCC_Callbacks::onAspectAntiHoraire(d[0]);
        break;

    case Cmd_CC_to_EXCC::DIRECTION_HORAIRE:
        if (frame.dlc < 1)
            break;
        EXCC_Callbacks::onDirectionHoraire(d[0]);
        break;

    case Cmd_CC_to_EXCC::DIRECTION_ANTIHORAIRE:
        if (frame.dlc < 1)
            break;
        EXCC_Callbacks::onDirectionAntiHoraire(d[0]);
        break;

    case Cmd_CC_to_EXCC::OCCUPATION_VOISINS:
        if (frame.dlc < 1)
            break;
        EXCC_Callbacks::onOccupationVoisins(d[0]);
        break;

    case Cmd_CC_to_EXCC::SERVO_MOVE:
        if (frame.dlc < 2)
            break;
        EXCC_Callbacks::onServoMove(d[0], d[1]);
        break;

    case Cmd_CC_to_EXCC::SERVO_CONFIG:
        if (frame.dlc < 7)
            break;
        EXCC_Callbacks::onServoConfig(
            d[0],
            uint16_t((d[1] << 8) | d[2]),
            uint16_t((d[3] << 8) | d[4]),
            uint16_t((d[5] << 8) | d[6]));
        break;

    case Cmd_CC_to_EXCC::SERVO_TEST:
        if (frame.dlc < 1)
            break;
        EXCC_Callbacks::onServoTest(d[0]);
        break;

    case Cmd_CC_to_EXCC::RECALIBRER_BOOSTER:
        EXCC_Callbacks::onRecalibrationBooster();
        break;

    case Cmd_CC_to_EXCC::SET_SEUILS:
        if (frame.dlc < 4)
            break;
        EXCC_Callbacks::onSetSeuils(
            uint16_t(d[0] | (d[1] << 8)),
            uint16_t(d[2] | (d[3] << 8)));
        break;

    case Cmd_CC_to_EXCC::BOOSTER_POWER:
        if (frame.dlc < 1)
            break;
        EXCC_Callbacks::onBoosterPower(d[0]);
        break;

    case Cmd_CC_to_EXCC::PROFILE_VOIE:
        if (frame.dlc < 1)
            break;
        EXCC_Callbacks::onProfileVoie(d[0]);
        break;

    default:
        // commande inconnue → ignorée
        break;
    }
}

void EXCC_CanCommands::begin()
{
    CanInit::begin(s_canConfig);
}

// ---------------------------------------------------------------------------
// Process — lit les trames CAN du CC et les traite
// ---------------------------------------------------------------------------
void EXCC_CanCommands::process()
{
    CanMsg msg;

    while (CanBus::bus(0).receive(msg))
    {
        if (!msg.is29())
            continue;

        s_lastRxMs = millis();
        s_ccSeen = true;
        uint8_t commande = msg.cmde();

        handleFrame(commande, msg);
    }
}

uint32_t EXCC_CanCommands::lastRxMs()
{
    return s_lastRxMs;
}

bool EXCC_CanCommands::hasReceivedFrame()
{
    return s_ccSeen;
}
