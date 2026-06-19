#include "EXCC_CanBooster.h"
#include "EXCC_CanConfig.h"
#include "CanInit.h"
#include "CanBus.h"
#include "EXCC_Booster.h"

/*
 * ============================================================================
 *  SECTION : Configuration CAN
 * ============================================================================
 *  Le bus CAN Booster utilise CanUniversal.
 *  EXCC_CanConfig contient :
 *      - vitesse
 *      - broches
 *      - type de driver (TWAI interne ou MCP2515)
 */
static EXCC_CanConfig canConfig;

/*
 * ============================================================================
 *  SECTION : État interne RailCom
 * ============================================================================
 *  Le cutout est annoncé par le Master via la trame CAN :
 *
 *      ID = 0x101
 *      data[0] = 1 → début cutout
 *      data[0] = 0 → fin cutout
 *
 *  EXCC doit :
 *      - appeler onCutoutStart() au début
 *      - appeler onCutoutEnd()   à la fin
 *      - activer feedRailcomSample() dans le timer HF
 */
static bool s_cutoutActive = false;

/*
 * ============================================================================
 *  EXCC_CanBooster::begin()
 * ============================================================================
 *  Initialise le bus CAN Booster.
 *
 *  CanInit::begin() :
 *      - détecte automatiquement le driver (TWAI / MCP2515)
 *      - configure la vitesse
 *      - prépare CanBus::bus(0) pour RX/TX
 */
void EXCC_CanBooster::begin()
{
    CanInit::begin(canConfig);
}

/*
 * ============================================================================
 *  EXCC_CanBooster::process()
 * ============================================================================
 *  Boucle de réception CAN.
 *
 *  Appelée toutes les 1 ms par EXCC_BoosterCore.
 *  Elle lit toutes les trames disponibles sur le bus 0.
 */
void EXCC_CanBooster::process()
{
    CanMsg msg;

    while (CanBus::bus(0).receive(msg))
    {
        handleFrame(msg);
    }
}

/*
 * ============================================================================
 *  EXCC_CanBooster::handleFrame()
 * ============================================================================
 *  Analyse et traitement d’une trame CAN Booster.
 *
 *  Rôle :
 *      - détecter le cutout (RailCom)
 *      - transmettre la trame à la bibliothèque CanDccBooster
 *
 *  IMPORTANT :
 *      La bibliothèque CanDccBooster gère :
 *          - DCC logique
 *          - cutout matériel
 *          - sécurité
 *          - télémétrie
 *
 *      MAIS elle n’appelle PAS les hooks RailCom :
 *          onCutoutStart()
 *          feedRailcomSample()
 *          onCutoutEnd()
 *
 *      → C’est EXCC qui doit les appeler.
 */
void EXCC_CanBooster::handleFrame(const CanMsg &msg)
{
    /*
     * ------------------------------------------------------------------------
     *  Détection du cutout RailCom (ID 0x101)
     * ------------------------------------------------------------------------
     *  Le Master envoie :
     *      data[0] = 1 → début cutout
     *      data[0] = 0 → fin cutout
     *
     *  EXCC doit déclencher les hooks RailCom de la bibliothèque.
     */
    if (msg.id == 0x101 && msg.dlc >= 1)
    {
        bool active = (msg.data[0] != 0);

        // Début du cutout
        if (active && !s_cutoutActive)
        {
            s_cutoutActive = true;
            EXCC_Booster::onCutoutStart();   // RailCom : début de fenêtre
        }

        // Fin du cutout
        else if (!active && s_cutoutActive)
        {
            s_cutoutActive = false;
            EXCC_Booster::onCutoutEnd();     // RailCom : fin + décodage
        }
    }

    /*
     * ------------------------------------------------------------------------
     *  Transmission de la trame à la bibliothèque CanDccBooster
     * ------------------------------------------------------------------------
     *  Le cœur logique gère :
     *      - DCC logique
     *      - cutout matériel
     *      - sécurité
     *      - télémétrie
     */
    EXCC_Booster::onCanMessage(msg.id, msg.data, msg.dlc);
}

/*
 * ============================================================================
 *  EXCC_CanBooster::isCutoutActive()
 * ============================================================================
 *  Fonction utilitaire utilisée par le timer HF RailCom.
 *
 *  Elle permet au timer d’appeler feedRailcomSample()
 *  uniquement pendant la fenêtre de cutout.
 */
bool EXCC_CanBooster::isCutoutActive()
{
    return s_cutoutActive;
}
