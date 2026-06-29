#include "EXCC_Ponctuel.h"
#include "EXCC_Compteur.h"
#include "EXCC_CAN_CC.h"

#include <Arduino.h>

bool EXCC_Ponctuel::s_actifH = false;
bool EXCC_Ponctuel::s_actifAH = false;

unsigned long EXCC_Ponctuel::s_lastH = 0;
unsigned long EXCC_Ponctuel::s_lastAH = 0;

extern uint8_t g_idCC;

void EXCC_Ponctuel::begin()
{
    s_actifH = false;
    s_actifAH = false;
}

void EXCC_Ponctuel::loop()
{
    unsigned long now = millis();

    // --- Côté H ---
    if (EXCC_Compteur::deltaH != 0)
    {
        activerCapteurH();
        s_lastH = now;
    }
    else if (s_actifH && (now - s_lastH > TIMEOUT_MS))
    {
        s_actifH = false;
        envoyerEtatCapteurH(false);
    }

    // --- Côté AH ---
    if (EXCC_Compteur::deltaAH != 0)
    {
        activerCapteurAH();
        s_lastAH = now;
    }
    else if (s_actifAH && (now - s_lastAH > TIMEOUT_MS))
    {
        s_actifAH = false;
        envoyerEtatCapteurAH(false);
    }
}

void EXCC_Ponctuel::activerCapteurH()
{
    if (!s_actifH)
    {
        s_actifH = true;
        envoyerEtatCapteurH(true);
    }
}

void EXCC_Ponctuel::activerCapteurAH()
{
    if (!s_actifAH)
    {
        s_actifAH = true;
        envoyerEtatCapteurAH(true);
    }
}

void EXCC_Ponctuel::envoyerEtatCapteurH(bool actif)
{
    EXCC_CAN_CC::envoyerPonctuelH(g_idCC, actif);
}

void EXCC_Ponctuel::envoyerEtatCapteurAH(bool actif)
{
    EXCC_CAN_CC::envoyerPonctuelAH(g_idCC, actif);
}
