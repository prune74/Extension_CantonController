#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>

/*
 * ============================================================================
 *  EXCC_Occupation
 *  --------------------------------------------------------------------------
 *  Module centralisé de gestion de l’occupation du canton côté EXCC.
 *
 *  Rôle :
 *    - Fusionner les différentes sources d’occupation :
 *         • Courant du booster
 *         • Compteur d’essieux (compteurGlobal)
 *         • Capteurs ponctuels H / AH
 *         • EA (occupation des voisins, envoyé par le CC)
 *
 *    - Produire un signal d’occupation final, propre, stable et fail-safe.
 *
 *    - Déclencher les actions associées :
 *         • Envoi UART vers le CC (PROTO_04_OCCUPATION)
 *         • Mise à jour de l’affichage local (cantonWS)
 *
 *  IMPORTANT :
 *    Ce module devient l’unique source de vérité pour l’occupation.
 * ============================================================================
 */
class EXCC_Occupation
{
public:
    // Initialisation interne
    static void begin();

    // Mise à jour depuis le courant mesuré par le booster
    static void majCourant(uint16_t courant_mA);

    // Mise à jour depuis le compteur d’essieux
    static void majCompteur(int compteurGlobal);

    // Mise à jour depuis les capteurs ponctuels (H / AH)
    static void majPonctuels(bool actifH, bool actifAH);

    // Mise à jour depuis EA (occupation des voisins)
    static void majVoisins(uint8_t EA);

    // Calcul final de l’occupation (à appeler en fin de loop EXCC)
    static void calculerOccupationFinale();

    // Lecture de l’état final (0 = libre, 1 = occupé)
    static bool estOccupe();

private:
    // Envoi UART + mise à jour LED canton
    static void appliquerChangementOccupation(bool occupe);

private:
    // États internes des différentes sources
    static bool s_occCourant;     // occupation par courant
    static bool s_occCompteur;    // occupation par compteurGlobal
    static bool s_occPonctuel;    // occupation par ponctuels H/AH

    // EA reçu du CC (SP1/SM1)
    static uint8_t s_voisinsEA;

    // Occupation finale stabilisée
    static bool s_occFinale;

    // Dernier état envoyé (pour éviter les répétitions)
    static bool s_dernierEnvoi;
};
