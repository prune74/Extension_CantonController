#pragma once

#include <stdint.h>
#include "EXCC_Quadrature.h"

/*
 * ============================================================
 *  EXCC_Compteur — Comptage d’essieux (Exploration 2026)
 * ------------------------------------------------------------
 *  Rôle :
 *    - Lire les événements quadrature H et AH
 *    - Détecter les deltas (+1 / -1)
 *    - Maintenir deux compteurs internes
 *    - Calculer le compteur global
 *    - Exposer deltaH / deltaAH pour EXCC_Ponctuel
 *
 *  Important :
 *    - Aucune ISR ici
 *    - Aucune logique RS485 ici
 *    - Ce module ne fait que compter
 * ============================================================
 */

class EXCC_Compteur
{
public:
    // Initialisation
    static void begin();

    // À appeler régulièrement dans la loop EXCC
    static void loop();

    // Reset complet (reboot EXCC)
    static void reset();

    // Accès aux compteurs
    static int compteurH();
    static int compteurAH();
    static int compteurGlobal();

    // Deltas détectés pendant la frame courante
    static int deltaH;
    static int deltaAH;

private:
    // États quadrature précédents
    static uint8_t s_prevH;
    static uint8_t s_prevAH;

    // Compteurs internes
    static int s_compteurH;
    static int s_compteurAH;

    // Lecture quadrature + calcul delta
    static void traiterQuadratureH();
    static void traiterQuadratureAH();

    // Machine d’état quadrature
    static int deltaFromQuadrature(uint8_t prev, uint8_t curr);
};
