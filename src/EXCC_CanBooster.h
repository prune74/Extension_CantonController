#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include <stddef.h>
#include "EXCC_Config.h"
#include "CanMsg.h"

/*
 * ============================================================================
 *  EXCC_CanBooster.h
 * ============================================================================
 *
 *  Rôle général :
 *  --------------
 *  Cette classe gère la communication CAN du Booster EXCC.
 *
 *  Elle sert d’interface entre :
 *      - le bus CAN Booster (DCC logique + cutout RailCom)
 *      - la bibliothèque CanDccBooster (cœur logique)
 *      - EXCC_Booster_WS2812 (wrapper Booster + LEDs + télémétrie)
 *
 *  Fonctionnement :
 *      - begin()   → initialise le bus CAN via CanUniversal
 *      - process() → lit toutes les trames CAN (appelée toutes les 1 ms)
 *      - handleFrame() → analyse chaque trame reçue
 *
 *  DCC entrant :
 *      0x100 contient un bloc compact : data[0] = nombre de bits,
 *      puis deux nibbles (bit/phase) par octet, premier nibble en bas.
 *      Les bits sont conservés dans une FIFO jusqu'à acceptation par RMT.
 *
 *  RailCom :
 *      0x101 transporte des marqueurs START/END. Leur espacement CAN
 *      ne représente pas la durée physique du cutout : EXCC utilise une
 *      fenêtre temporisée localement.
 */

class EXCC_CanBooster
{
public:
    /*
     * ------------------------------------------------------------------------
     *  begin()
     * ------------------------------------------------------------------------
     *  Initialise le bus CAN Booster (driver, vitesse, broches…)
     */
    static void begin();

    /*
     * ------------------------------------------------------------------------
     *  process()
     * ------------------------------------------------------------------------
     *  Lit toutes les trames CAN disponibles.
     *  Appelée toutes les 1 ms par EXCC_BoosterCore.
     */
    static void process();

    /*
     * ------------------------------------------------------------------------
     *  isCutoutActive()
     * ------------------------------------------------------------------------
     *  Indique si la fenêtre RailCom est active.
     *
     *  Utilisé par le timer HF pour appeler feedRailcomSample()
     *  uniquement pendant le cutout.
     */
    static bool isCutoutActive();
    static bool isCutoutPending();
    static void startPendingCutout();

    /*
     * ------------------------------------------------------------------------
     *  lastCanRxMs()
     * ------------------------------------------------------------------------
     *  Timestamp de la dernière trame CAN reçue.
     *  Utilisé pour la LED CAN (WS2812 Status).
     */
    static uint32_t lastCanRxMs();

    /*
     * ------------------------------------------------------------------------
     *  lastCutoutMs()
     * ------------------------------------------------------------------------
     *  Timestamp du dernier cutout (début ou fin).
     *  Utilisé pour la LED RailCom (état global du cutout).
     */
    static uint32_t lastCutoutMs();

    /*
     * BENCH/DIAG : nombre total de trames CAN reçues depuis le boot.
     */
    static uint32_t rxCount();

    // Lecture sans consommation, puis consommation après acceptation RMT.
    static size_t peekDccBits(uint8_t *bits, size_t capacity);
    static void consumeDccBits(size_t count);
    static void discardDccBits();

private:
    /*
     * ------------------------------------------------------------------------
     *  handleFrame()
     * ------------------------------------------------------------------------
     *  Analyse une trame CAN Booster :
     *      - dépaquette les blocs DCC (ID 0x100)
     *      - démarre la fenêtre cutout locale (ID 0x101)
     *      - transmet les autres trames au booster
     */
    static void handleFrame(const CanMsg &msg);
};
