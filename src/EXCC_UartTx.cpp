#include "EXCC_UartTx.h"
#include "EXCC_RS485.h"
#include "Exploration_Protocol.h"

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

/*
 * EXCC_UartTx.cpp — Trames UART envoyées par l’EXCC vers le CC.
 *
 * Protocole 2026 :
 *   [SYNC=0xAA][OPCODE][DATA...]
 *
 * L’EXCC est unique → aucune notion d’index.
 */

/* ============================================================
 * envoyerPong()
 * ============================================================ */
void EXCC_UartTx::envoyerPong()
{
    uint8_t frame[2] = {
        PROTO_SYNC_BYTE,
        PROTO_PONG
    };

    EXCC_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerPonctuelH()
 * ============================================================ */
void EXCC_UartTx::envoyerPonctuelH(bool actif)
{
    uint8_t code = actif ? PROTO_PONCT_H_ACTIVE : PROTO_PONCT_H_INACTIVE;

    uint8_t frame[3] = {
        PROTO_SYNC_BYTE,
        PROTO_03_H_PONCTUEL,
        code
    };

    EXCC_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerPonctuelAH()
 * ============================================================ */
void EXCC_UartTx::envoyerPonctuelAH(bool actif)
{
    uint8_t code = actif ? PROTO_PONCT_AH_ACTIVE : PROTO_PONCT_AH_INACTIVE;

    uint8_t frame[3] = {
        PROTO_SYNC_BYTE,
        PROTO_03_AH_PONCTUEL,
        code
    };

    EXCC_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerOccupation()
 * ============================================================ */
void EXCC_UartTx::envoyerOccupation(bool occ)
{
    uint8_t code = occ ? PROTO_OCC_ACTIVE : PROTO_OCC_LIBRE;

    uint8_t frame[3] = {
        PROTO_SYNC_BYTE,
        PROTO_04_OCCUPATION,
        code
    };

    EXCC_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerBooster()
 * ============================================================ */
void EXCC_UartTx::envoyerBooster(uint8_t etat,
                                 uint8_t courant,
                                 uint8_t tension)
{
    uint8_t frame[5] = {
        PROTO_SYNC_BYTE,
        PROTO_07_BOOSTER,
        etat,
        courant,
        tension
    };

    EXCC_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerRailcom()
 * ============================================================ */
void EXCC_UartTx::envoyerRailcom(uint8_t type,
                                 uint16_t adresse)
{
    uint8_t frame[5] = {
        PROTO_SYNC_BYTE,
        PROTO_08_RAILCOM_ADRESSE,
        type,
        uint8_t(adresse & 0xFF),
        uint8_t((adresse >> 8) & 0xFF)
    };

    EXCC_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerCalibBooster()
 * ============================================================ */
void EXCC_UartTx::envoyerCalibBooster(uint16_t seuilLibre_mA,
                                      uint16_t seuilOccupe_mA)
{
    uint8_t frame[6] = {
        PROTO_SYNC_BYTE,
        PROTO_09_CALIB_BOOSTER,
        uint8_t(seuilLibre_mA & 0xFF),
        uint8_t(seuilLibre_mA >> 8),
        uint8_t(seuilOccupe_mA & 0xFF),
        uint8_t(seuilOccupe_mA >> 8)
    };

    EXCC_RS485::send(frame, sizeof(frame));
}

/* ============================================================
 * envoyerPositionAiguille()
 * ============================================================ */
void EXCC_UartTx::envoyerPositionAiguille(uint8_t servoIndex,
                                          uint8_t position,
                                          uint8_t etat)
{
    uint8_t frame[5] = {
        PROTO_SYNC_BYTE,
        PROTO_06_POSITION_AIGUILLE,
        servoIndex,
        position,
        etat
    };

    EXCC_RS485::send(frame, sizeof(frame));
}
