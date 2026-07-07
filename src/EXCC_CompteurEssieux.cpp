#include "EXCC_CompteurEssieux.h"
#include "EXCC_Compteur.h"
#include "EXCC_CAN_CC.h"

uint16_t EXCC_CompteurEssieux::s_idCanton = 0;

bool EXCC_CompteurEssieux::s_countingH = false;
bool EXCC_CompteurEssieux::s_countingAH = false;
bool EXCC_CompteurEssieux::s_finishedFirst = false;

void EXCC_CompteurEssieux::begin(uint16_t idCanton)
{
    s_idCanton = idCanton;
    s_countingH = false;
    s_countingAH = false;
    s_finishedFirst = false;
}

void EXCC_CompteurEssieux::loop()
{
    detectSens();
    detectFinComptage();
    detectFinTrain();
}

void EXCC_CompteurEssieux::detectSens()
{
    if (!s_countingH && !s_countingAH)
    {
        if (EXCC_Compteur::deltaH != 0)
            s_countingH = true;   // sens horaire

        if (EXCC_Compteur::deltaAH != 0)
            s_countingAH = true;  // sens antihoraire
    }
}

void EXCC_CompteurEssieux::detectFinComptage()
{
    if (s_countingH && !s_finishedFirst && EXCC_Compteur::deltaAH != 0)
    {
        uint8_t essieux = EXCC_Compteur::compteurH() / 4;
        envoyerEssieux(essieux);
        s_finishedFirst = true;
    }

    if (s_countingAH && !s_finishedFirst && EXCC_Compteur::deltaH != 0)
    {
        uint8_t essieux = EXCC_Compteur::compteurAH() / 4;
        envoyerEssieux(essieux);
        s_finishedFirst = true;
    }
}

void EXCC_CompteurEssieux::detectFinTrain()
{
    if (s_finishedFirst && EXCC_Compteur::compteurGlobal() == 0)
    {
        EXCC_Compteur::reset();
        s_countingH = false;
        s_countingAH = false;
        s_finishedFirst = false;
    }
}

void EXCC_CompteurEssieux::envoyerEssieux(uint8_t essieux)
{
    EXCC_CAN_CC::envoyerEssieux(s_idCanton, essieux);
}
