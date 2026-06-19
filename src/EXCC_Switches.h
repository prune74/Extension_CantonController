#pragma once
#include <stdint.h>
#include <Adafruit_MCP23X17.h>
#include "EXCC_Config.h"

/*
 *  EXCC_Switches.h — Gestion des micro-switchs d’aiguilles via MCP23017
 */

class EXCC_Switches
{
public:
    static void begin();
    static void update();
    static void notifierMouvementDemarre(uint8_t idx);

private:
    static Adafruit_MCP23X17 mcp;

    static const uint8_t swDroit[AIG_COUNT];
    static const uint8_t swDevie[AIG_COUNT];

    static uint8_t lastPos[AIG_COUNT];
    static uint8_t lastEtat[AIG_COUNT];
    static uint32_t lastSendMs[AIG_COUNT];

    static uint32_t moveStartMs[AIG_COUNT];
    static uint32_t moveMaxMs[AIG_COUNT];
    static bool movementActive[AIG_COUNT];

    static uint8_t lirePosition(uint8_t idx);
    static uint8_t lireEtat(uint8_t idx, uint8_t pos);
    static void envoyerTrame(uint8_t idx, uint8_t pos, uint8_t etat);
};
