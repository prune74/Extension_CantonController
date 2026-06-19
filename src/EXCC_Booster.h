#pragma once
#include <stdint.h>

/*
 * ============================================================================
 *  EXCC_Booster.h — Interface Booster EXCC (Gestion Canton 2026)
 * ============================================================================
 *
 *  Rôle :
 *  ------
 *  Cette classe est la façade EXCC autour du cœur Booster :
 *
 *      - EXCC_BoosterHardware  → accès matériel bas niveau
 *      - BoosterConfig         → paramètres dynamiques
 *      - CanDccBooster         → logique DCC + RailCom + sécurité
 *
 *  Elle fournit une API simple au reste du firmware :
 *      • activation / désactivation voie
 *      • mise à jour 1 kHz (DCC, RailCom, sécurité)
 *      • réception trames CAN Booster
 *      • publication télémétrie (courant, tension, fautes)
 *      • gestion occupation canton (via courant consommé)
 *
 *  Notes :
 *  -------
 *  - Le décodage RailCom HF est effectué par CanDccBooster.
 *  - EXCC doit simplement relayer :
 *        → onCutoutStart()
 *        → onCutoutEnd()
 *        → feedRailcomSample()
 *
 *  - La logique d’occupation est spécifique EXCC (courant + hystérésis).
 * ============================================================================
 */

class EXCC_Booster
{
public:
    /*
     * ------------------------------------------------------------------------
     *  begin()
     * ------------------------------------------------------------------------
     *  Initialise :
     *      - le hardware EXCC (MOSFET, ADC, protections…)
     *      - la configuration Booster
     *      - la bibliothèque CanDccBooster
     */
    static void begin();

    /*
     * ------------------------------------------------------------------------
     *  update()
     * ------------------------------------------------------------------------
     *  Appelée toutes les 1 ms par EXCC_BoosterCore.
     *
     *  Rôle :
     *      - exécuter la logique DCC + RailCom
     *      - mettre à jour la télémétrie
     *      - transmettre l’adresse RailCom détectée
     *      - mettre à jour l’occupation du canton
     */
    static void update();

    /*
     * ------------------------------------------------------------------------
     *  setEnabled()
     * ------------------------------------------------------------------------
     *  Active ou désactive complètement la sortie voie.
     *  (Coupure immédiate + reset télémétrie + canton = LIBRE)
     */
    static void setEnabled(bool enabled);

    /*
     * ------------------------------------------------------------------------
     *  onCanMessage()
     * ------------------------------------------------------------------------
     *  Appelé par EXCC_CanBooster pour chaque trame CAN_BOOSTER reçue.
     *  Transmet la trame à la bibliothèque CanDccBooster.
     */
    static void onCanMessage(uint32_t id, const uint8_t *data, uint8_t len);

    /*
     * ------------------------------------------------------------------------
     *  API télémétrie
     * ------------------------------------------------------------------------
     */
    static uint16_t readCurrent_mA();
    static uint16_t readVoltage_mV();
    static bool isThermalFault();

    /*
     * ------------------------------------------------------------------------
     *  HOOKS RAILCOM (Gestion Canton 2026)
     * ------------------------------------------------------------------------
     *  Appelés par :
     *      - EXCC_CanBooster (début/fin cutout)
     *      - le timer HF (échantillonnage RailCom)
     *
     *  Ils relaient les appels vers CanDccBooster.
     */
    static void onCutoutStart();
    static void onCutoutEnd();
    static void feedRailcomSample();

private:
    /*
     * ------------------------------------------------------------------------
     *  updateOccupation()
     * ------------------------------------------------------------------------
     *  Détection de l’occupation du canton via le courant consommé.
     *  Utilise les seuils calibrés + hystérésis.
     */
    static void updateOccupation(uint16_t courant_mA);
};
