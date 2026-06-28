#include "EXCC_CanCommands.h"
#include "EXCC_Callbacks.h"
#include "Protocol.h"
#include "CanBus.h"
#include "CanMsg.h"

// ---------------------------------------------------------------------------
// Traitement d’une commande CC → EXCC
// ---------------------------------------------------------------------------
static void handleFrame(uint8_t commande, const CanMsg &frame)
{
    const uint8_t *d = frame.data;

    switch (Cmd_CC_to_EXCC(commande))
    {
    case Cmd_CC_to_EXCC::CANTON_ID:
        EXCC_Callbacks::onCantonID(d, frame.dlc);
        break;

    case Cmd_CC_to_EXCC::PING:
        EXCC_Callbacks::onPing();
        break;

    case Cmd_CC_to_EXCC::CONFIG_SIGNAUX:
        EXCC_Callbacks::onConfigSignaux(d, frame.dlc);
        break;

    case Cmd_CC_to_EXCC::ASPECT_HORAIRE:
        EXCC_Callbacks::onAspectHoraire(d[0]);
        break;

    case Cmd_CC_to_EXCC::ASPECT_ANTIHORAIRE:
        EXCC_Callbacks::onAspectAntiHoraire(d[0]);
        break;

    case Cmd_CC_to_EXCC::DIRECTION_HORAIRE:
        EXCC_Callbacks::onDirectionHoraire(d[0]);
        break;

    case Cmd_CC_to_EXCC::DIRECTION_ANTIHORAIRE:
        EXCC_Callbacks::onDirectionAntiHoraire(d[0]);
        break;

    case Cmd_CC_to_EXCC::OCCUPATION_VOISINS:
        EXCC_Callbacks::onOccupationVoisins(d[0]);
        break;

    case Cmd_CC_to_EXCC::SERVO_MOVE:
        EXCC_Callbacks::onServoMove(d[0], d[1]);
        break;

    case Cmd_CC_to_EXCC::SERVO_CONFIG:
        EXCC_Callbacks::onServoConfig(
            d[0],
            uint16_t(d[1] | (d[2] << 8)),
            uint16_t(d[3] | (d[4] << 8)),
            uint16_t(d[5] | (d[6] << 8)));
        break;

    case Cmd_CC_to_EXCC::SERVO_TEST:
        EXCC_Callbacks::onServoTest(d[0]);
        break;

    case Cmd_CC_to_EXCC::RECALIBRER_BOOSTER:
        EXCC_Callbacks::onRecalibrationBooster();
        break;

    case Cmd_CC_to_EXCC::SET_SEUILS:
        EXCC_Callbacks::onSetSeuils(
            uint16_t(d[0] | (d[1] << 8)),
            uint16_t(d[2] | (d[3] << 8)));
        break;

    case Cmd_CC_to_EXCC::BOOSTER_POWER:
        EXCC_Callbacks::onBoosterPower(d[0]);
        break;

    case Cmd_CC_to_EXCC::PROFILE_VOIE:
        EXCC_Callbacks::onProfileVoie(d[0]);
        break;

    default:
        // commande inconnue → ignorée
        break;
    }
}

// ---------------------------------------------------------------------------
// Process — lit les trames CAN du CC et les traite
// ---------------------------------------------------------------------------
void EXCC_CanCommands::process()
{
    CanMsg msg;

    while (CanBus::bus(1).receive(msg))
    {
        if (!msg.is29())
            continue;

        uint8_t commande = msg.cmde();

        handleFrame(commande, msg);
    }
}
