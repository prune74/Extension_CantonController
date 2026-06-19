#include "EXCC_Ponctuel.h"
#include "EXCC_Compteur.h"
#include "EXCC_UartTx.h"

#include <Arduino.h>

bool EXCC_Ponctuel::s_actifH  = false;
bool EXCC_Ponctuel::s_actifAH = false;

unsigned long EXCC_Ponctuel::s_lastH  = 0;
unsigned long EXCC_Ponctuel::s_lastAH = 0;

void EXCC_Ponctuel::begin()
{
    s_actifH  = false;
    s_actifAH = false;
}

void EXCC_Ponctuel::loop()
{
    unsigned long now = millis();

    // --- Côté H ---
    if (EXCC_Compteur::deltaH != 0)
    {
        activerH();
        s_lastH = now;
    }
    else if (s_actifH && (now - s_lastH > TIMEOUT_MS))
    {
        s_actifH = false;
        envoyerH(false);
    }

    // --- Côté AH ---
    if (EXCC_Compteur::deltaAH != 0)
    {
        activerAH();
        s_lastAH = now;
    }
    else if (s_actifAH && (now - s_lastAH > TIMEOUT_MS))
    {
        s_actifAH = false;
        envoyerAH(false);
    }
}

void EXCC_Ponctuel::activerH()
{
    if (!s_actifH)
    {
        s_actifH = true;
        envoyerH(true);
    }
}

void EXCC_Ponctuel::activerAH()
{
    if (!s_actifAH)
    {
        s_actifAH = true;
        envoyerAH(true);
    }
}

void EXCC_Ponctuel::envoyerH(bool actif)
{
    EXCC_UartTx::envoyerPonctuelH(actif);
}

void EXCC_Ponctuel::envoyerAH(bool actif)
{
    EXCC_UartTx::envoyerPonctuelAH(actif);
}
