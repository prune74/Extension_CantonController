#include "EXCC_UartRx.h"
#include "EXCC_RS485.h"
#include "EXCC_Pins.h"
#include "Exploration_Protocol.h"
#include "EXCC_Callbacks.h"

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

static HardwareSerial *uart = nullptr;

/* ============================================================
   Buffer interne du parser
============================================================ */
static uint8_t rxState  = 0;
static uint8_t rxOpcode = 0;
static uint8_t rxData[32];
static uint8_t rxPos    = 0;

/* ============================================================
   resetParser()
============================================================ */
static inline void resetParser() noexcept
{
    rxState  = 0;
    rxOpcode = 0;
    rxPos    = 0;
}

/* ============================================================
   handleFrame()
   Dispatch vers EXCC_Callbacks selon l’opcode reçu.
============================================================ */
static inline void handleFrame(uint8_t opcode, uint8_t *data, uint8_t len) noexcept
{
    switch (opcode)
    {
    case PROTO_PING:
        // EXCC_UartTx::envoyerPong();
        break;

    /* ============================
       E4 — Topologie
    ============================ */
    case PROTO_E4_TOPOLOGIE_CAN:
        EXCC_Callbacks::onTopologie(data, len);
        break;

    /* ============================
       E5 — Configuration signaux
    ============================ */
    case PROTO_E5_CONFIG_SIGNAUX:
        EXCC_Callbacks::onConfigSignaux(data, len);
        break;

    /* ============================
       E6 / E7 — Aspects SNCF
    ============================ */
    case PROTO_E6_ASPECT_HORAIRE:
        EXCC_Callbacks::onAspectHoraire(data[0]);
        break;

    case PROTO_E7_ASPECT_ANTIHORAIRE:
        EXCC_Callbacks::onAspectAntiHoraire(data[0]);
        break;

    /* ============================
       E8 / E9 — Direction LEDs
    ============================ */
    case PROTO_E8_DIRECTION_HORAIRE:
        EXCC_Callbacks::onDirectionHoraire(data[0]);
        break;

    case PROTO_E9_DIRECTION_ANTIHORAIRE:
        EXCC_Callbacks::onDirectionAntiHoraire(data[0]);
        break;

    /* ============================
       EA — Occupation voisins
    ============================ */
    case PROTO_EA_OCCUPATION_VOISINS:
        EXCC_Callbacks::onOccupationVoisins(data[0]);
        break;

    /* ============================
       F0 — Servo Move
    ============================ */
    case PROTO_F0_SERVO_MOVE:
        EXCC_Callbacks::onServoMove(data[0], data[1]);
        break;

    /* ============================
       F1 — Servo Config
    ============================ */
    case PROTO_F1_SERVO_CONFIG:
    {
        uint8_t idx = data[0];
        uint16_t posDroit = data[1] | (data[2] << 8);
        uint16_t posDevie = data[3] | (data[4] << 8);
        uint16_t speed    = data[5] | (data[6] << 8);
        EXCC_Callbacks::onServoConfig(idx, posDroit, posDevie, speed);
        break;
    }

    /* ============================
       F2 — Servo Test
    ============================ */
    case PROTO_F2_SERVO_TEST:
        EXCC_Callbacks::onServoTest(data[0]);
        break;

    /* ============================
       F3 — Recalibration Booster
    ============================ */
    case PROTO_F3_RECALIBRER_BOOSTER:
        EXCC_Callbacks::onRecalibrationBooster();
        break;

    /* ============================
       F4 — Set Seuils Booster
    ============================ */
    case PROTO_F4_SET_SEUILS:
    {
        uint16_t libre  = data[0] | (data[1] << 8);
        uint16_t occupe = data[2] | (data[3] << 8);
        EXCC_Callbacks::onSetSeuils(libre, occupe);
        break;
    }

    /* ============================
       F5 — Booster Power ON/OFF
    ============================ */
    case PROTO_F5_BOOSTER_POWER:
        EXCC_Callbacks::onBoosterPower(data[0]);
        break;

    default:
        break;
    }
}

/* ============================================================
   begin()
============================================================ */
void EXCC_UartRx::begin(HardwareSerial &serial, uint32_t baudrate) noexcept
{
    EXCC_RS485::begin(serial, baudrate, PIN_RS485_DE_RE);
    uart = &EXCC_RS485::uart();
}

/* ============================================================
   process()
   Parsing non bloquant du protocole CC → EXCC
============================================================ */
void EXCC_UartRx::process() noexcept
{
    if (!uart)
        return;

    while (uart->available())
    {
        const uint8_t b = uart->read();

        switch (rxState)
        {
        case 0: // SYNC
            if (b == PROTO_SYNC_BYTE)
                rxState = 1;
            break;

        case 1: // OPCODE
            rxOpcode = b;
            rxPos = 0;
            rxState = 2;
            break;

        case 2: // DATA
        {
            rxData[rxPos++] = b;

            uint8_t expectedLen = 0;

            switch (rxOpcode)
            {
            case PROTO_PING: expectedLen = 0; break;
            case PROTO_E5_CONFIG_SIGNAUX: expectedLen = 1; break;
            case PROTO_E6_ASPECT_HORAIRE: expectedLen = 1; break;
            case PROTO_E7_ASPECT_ANTIHORAIRE: expectedLen = 1; break;
            case PROTO_E8_DIRECTION_HORAIRE: expectedLen = 1; break;
            case PROTO_E9_DIRECTION_ANTIHORAIRE: expectedLen = 1; break;
            case PROTO_EA_OCCUPATION_VOISINS: expectedLen = 1; break;
            case PROTO_F0_SERVO_MOVE: expectedLen = 2; break;
            case PROTO_F1_SERVO_CONFIG: expectedLen = 7; break;
            case PROTO_F2_SERVO_TEST: expectedLen = 1; break;
            case PROTO_F3_RECALIBRER_BOOSTER: expectedLen = 0; break;
            case PROTO_F4_SET_SEUILS: expectedLen = 4; break;
            case PROTO_F5_BOOSTER_POWER: expectedLen = 1; break;

            case PROTO_E4_TOPOLOGIE_CAN:
                if (rxPos < 3)
                    break;
                expectedLen = 3 + rxData[1] + rxData[2];
                break;

            default:
                expectedLen = 0;
                break;
            }

            if (rxPos >= expectedLen)
            {
                handleFrame(rxOpcode, rxData, expectedLen);
                resetParser();
            }
            break;
        }

        default:
            resetParser();
            break;
        }
    }
}
