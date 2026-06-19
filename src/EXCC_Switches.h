#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include <Adafruit_MCP23X17.h>
#include "EXCC_Config.h"

/*
 * ============================================================
 *  EXCC_Switches.h — Gestion des micro-switchs d’aiguilles
 *  + sécurité anti-blocage (version Discovery 2026)
 * ------------------------------------------------------------
 *  Rôle :
 *    - Lecture des micro-switchs via MCP23017
 *    - Déduction position réelle : DROIT / DEVIÉ / INDET / INCOHÉRENT
 *    - Déduction état sécurité   : OK / BLOQUÉ / ERREUR
 *    - Détection blocage dynamique
 *    - Envoi trame 0x06 au CC
 *
 *  Nouveautés 2026 :
 *    - Helpers pour LED Switches (diagnostic 4D répétitif) :
 *         • getWorstState()
 *         • anyMovement()
 *
 *  Contraintes :
 *    - Aucun malloc
 *    - Lecture non bloquante
 *    - update() appelé ~1 kHz
 * ============================================================
 */

class EXCC_Switches
{
public:
    // Initialisation du MCP23017
    static void begin();

    // Boucle principale (lecture + sécurité + envoi trames)
    static void update();

    // Appelé par EXCC_Servo lorsqu’un mouvement commence
    static void notifierMouvementDemarre(uint8_t idx);

    // Helpers pour EXCC_StatusLed (LED Switches 4D)
    static uint8_t getWorstState();   // OK / ERREUR / BLOQUE
    static bool anyMovement();        // au moins une aiguille en mouvement

private:
    // Driver MCP23017
    static Adafruit_MCP23X17 mcp;

    // Index des micro-switchs (définis dans le .cpp)
    static const uint8_t swDroit[AIG_COUNT];
    static const uint8_t swDevie[AIG_COUNT];

    // Derniers états envoyés au CC
    static uint8_t  lastPos[AIG_COUNT];
    static uint8_t  lastEtat[AIG_COUNT];
    static uint32_t lastSendMs[AIG_COUNT];

    // Suivi du mouvement pour sécurité anti-blocage
    static uint32_t moveStartMs[AIG_COUNT];
    static uint32_t moveMaxMs[AIG_COUNT];
    static bool     movementActive[AIG_COUNT];

    // Lecture brute des micro-switchs
    static uint8_t lirePosition(uint8_t idx);

    // Déduction état sécurité (OK / ERREUR / BLOQUE)
    static uint8_t lireEtat(uint8_t idx, uint8_t pos);

    // Envoi trame 0x06 au CC
    static void envoyerTrame(uint8_t idx, uint8_t pos, uint8_t etat);
};
