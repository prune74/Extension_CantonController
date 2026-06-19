#pragma once
#include <stdint.h>
#include <HardwareSerial.h>

/*
 * EXCC_UartRx.h — Réception et parsing du protocole CC → EXCC (RS485)
 *
 * Fonctionnement :
 *   - Le CC envoie des trames structurées :
 *        SYNC (0xAA)
 *        OPCODE
 *        DATA[n]
 *
 *   - Le parser fonctionne comme une machine à états :
 *        0 → attente SYNC
 *        1 → lecture OPCODE
 *        2 → lecture DATA
 *
 *   - Une fois la trame complète :
 *        → dispatch vers EXCC_Callbacks (à créer)
 *
 * Contraintes :
 *   - Aucun malloc
 *   - Parsing non bloquant
 *   - Compatible ISR RS485 (via EXCC_RS485)
 *   - Très robuste aux erreurs
 */

class EXCC_UartRx
{
public:
    static void begin(HardwareSerial &serial, uint32_t baudrate) noexcept;
    static void process() noexcept;
};
