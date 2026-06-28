#include "EXCC_CAN_CC.h"
#include "Protocol.h"
#include "Protocol.h"
#include "CanBus.h"
#include "CanMsg.h"
#include "CanID.h"

static void sendEXCC(uint8_t prio, uint16_t cmd, uint8_t resp,
                     uint16_t idCanton, const uint8_t *data, uint8_t len)
{
    uint32_t id29 = CanID::make29(prio, cmd, resp, idCanton);

    CanMsg msg(id29, {});
    msg.dlc = len;

    for (uint8_t i = 0; i < len; i++)
        msg.data[i] = data[i];

    CanBus::bus(1).send(msg);
}

void EXCC_CAN_CC::sendPong(uint16_t idCanton)
{
    sendEXCC(1, (uint16_t)Cmd_EXCC_to_CC::PONG, 0, idCanton, nullptr, 0);
}

void EXCC_CAN_CC::envoyerPonctuelH(uint16_t idCanton, bool actif)
{
    uint8_t d[1] = { static_cast<uint8_t>(actif ? 1 : 0) };
    sendEXCC(1, (uint16_t)Cmd_EXCC_to_CC::PONCTUEL_H, 0, idCanton, d, 1);
}

void EXCC_CAN_CC::envoyerPonctuelAH(uint16_t idCanton, bool actif)
{
    uint8_t d[1] = { static_cast<uint8_t>(actif ? 1 : 0) };
    sendEXCC(1, (uint16_t)Cmd_EXCC_to_CC::PONCTUEL_AH, 0, idCanton, d, 1);
}

void EXCC_CAN_CC::envoyerOccupation(uint16_t idCanton, bool occ)
{
    uint8_t d[1] = { static_cast<uint8_t>(occ ? 1 : 0) };
    sendEXCC(1, (uint16_t)Cmd_EXCC_to_CC::OCCUPATION, 0, idCanton, d, 1);
}

void EXCC_CAN_CC::envoyerBooster(uint16_t idCanton, uint8_t etat, uint8_t courant, uint8_t tension)
{
    uint8_t d[3] = {etat, courant, tension};
    sendEXCC(1, (uint16_t)Cmd_EXCC_to_CC::BOOSTER_INFO, 0, idCanton, d, 3);
}

void EXCC_CAN_CC::envoyerRailcom(uint16_t idCanton, uint8_t type, uint16_t adresse)
{
    uint8_t d[3] = {type, uint8_t(adresse & 0xFF), uint8_t(adresse >> 8)};
    sendEXCC(1, (uint16_t)Cmd_EXCC_to_CC::RAILCOM_ADRESSE, 0, idCanton, d, 3);
}

void EXCC_CAN_CC::envoyerCalibBooster(uint16_t idCanton, uint16_t libre, uint16_t occupe)
{
    uint8_t d[4] = {
        uint8_t(libre & 0xFF), uint8_t(libre >> 8),
        uint8_t(occupe & 0xFF), uint8_t(occupe >> 8)};
    sendEXCC(1, (uint16_t)Cmd_EXCC_to_CC::CALIB_BOOSTER_INFO, 0, idCanton, d, 4);
}

void EXCC_CAN_CC::envoyerPositionAiguille(uint16_t idCanton, uint8_t servoIndex, uint8_t position, uint8_t etat)
{
    uint8_t d[3] = {servoIndex, position, etat};
    sendEXCC(1, (uint16_t)Cmd_EXCC_to_CC::POSITION_AIGUILLE, 0, idCanton, d, 3);
}
