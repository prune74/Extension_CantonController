#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "Exploration_Protocol.h"

/*
 * EXCC_UartTx.h — Gestion des trames UART envoyées par l’EXCC vers le CC.
 *
 * Toutes les trames suivent le protocole Gestion Canton 2026 :
 *   SYNC (0xAA)
 *   OPCODE
 *   DATA[n]
 *
 * RÈGLE OFFICIELLE (Gestion Canton 2026 — RS485 multi‑EXCC + EXCC) :
 * ------------------------------------------------------------
 *   → Les trames EXCC → CC commencent par :
 *
 *        [SYNC][OPCODE][DATA...]
 *
 *     Contrairement à l’EXCC, l’EXCC n’a PAS d’adresse 0/1.
 *     Il s’agit d’une carte unique, non dupliquée.
 *
 * L’EXCC envoie :
 *   - PONG
 *   - PONCTUEL H
 *   - PONCTUEL AH
 *   - COMPTEUR ESSIEUX GLOBAL
 *   - OCCUPATION (si booster EXCC)
 *   - BOOSTER (état + courant + tension)
 *   - RAILCOM (adresse détectée)
 *   - CALIBRATION BOOSTER (seuilLibre + seuilOccupe)
 */

class EXCC_UartTx
{
public:
    /* --- Supervision --- */
    static void envoyerPong();

    /* --- Capteurs ponctuels H / AH --- */
    static void envoyerPonctuelH(bool actif);
    static void envoyerPonctuelAH(bool actif);

    /* --- Compteur essieux global --- */
    static void envoyerCompteurGlobal(uint8_t compteur);

    /* --- Occupation canton (booster EXCC) --- */
    static void envoyerOccupation(bool occ);

    /* --- Booster : état + courant + tension ---
     *
     * Trame envoyée (5 octets) :
     *   [0] SYNC       = 0xAA
     *   [1] OPCODE     = PROTO_07_BOOSTER
     *   [2] etat       = état du booster :
     *                      0 = OFF
     *                      1 = OK
     *                      2 = FAULT
     *                      3 = OVERHEAT
     *   [3] courant    = courant mesuré (x10 mA)
     *   [4] tension    = tension mesurée (x100 mV)
     */
    static void envoyerBooster(uint8_t etat,
                               uint8_t courant,
                               uint8_t tension);

    /* --- RailCom : type + adresse ---
     *
     * Trame envoyée (5 octets) :
     *   [0] SYNC
     *   [1] OPCODE
     *   [2] type
     *   [3] adresse LSB
     *   [4] adresse MSB
     */
    static void envoyerRailcom(uint8_t type,
                               uint16_t adresse);

    /* --- Calibration Booster : seuilLibre + seuilOccupe ---
     *
     * Trame envoyée (5 octets) :
     *   [0] SYNC
     *   [1] OPCODE
     *   [2] LIBRE_LSB
     *   [3] LIBRE_MSB
     *   [4] OCCUPE_LSB
     *   [5] OCCUPE_MSB
     */
    static void envoyerCalibBooster(uint16_t seuilLibre_mA,
                                    uint16_t seuilOccupe_mA);

    /* --- Position réelle des aiguilles ---
     *
     * Trame envoyée (5 octets) :
     *   [0] SYNC
     *   [1] OPCODE = PROTO_06_POSITION_AIGUILLE
     *   [2] index
     *   [3] position
     *   [4] etat
     */
    static void envoyerPositionAiguille(uint8_t index,
                                        uint8_t position,
                                        uint8_t etat);
};
