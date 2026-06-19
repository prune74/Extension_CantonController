#pragma once
#include <stdint.h>
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
 *      - EXCC_Booster (wrapper EXCC)
 *
 *  Fonctionnement :
 *      - begin()   → initialise le bus CAN via CanUniversal
 *      - process() → lit toutes les trames CAN (appelée toutes les 1 ms)
 *      - handleFrame() → analyse chaque trame reçue
 *
 *  Gestion RailCom :
 *      Le cutout RailCom est annoncé par le Master via la trame :
 *
 *          ID = 0x101
 *          data[0] = 1 → début cutout
 *          data[0] = 0 → fin cutout
 *
 *      EXCC_CanBooster doit :
 *          - appeler EXCC_Booster::onCutoutStart()
 *          - appeler EXCC_Booster::onCutoutEnd()
 *          - exposer isCutoutActive() pour le timer HF RailCom
 *
 *  NOTE :
 *      La bibliothèque CanDccBooster gère le cutout matériel,
 *      mais ne déclenche PAS elle-même le décodage RailCom HF.
 *      C’est EXCC qui doit appeler les hooks RailCom.
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

private:
    /*
     * ------------------------------------------------------------------------
     *  handleFrame()
     * ------------------------------------------------------------------------
     *  Analyse une trame CAN Booster :
     *      - détecte le cutout (ID 0x101)
     *      - appelle les hooks RailCom
     *      - transmet la trame à EXCC_Booster
     */
    static void handleFrame(const CanMsg &msg);
};
