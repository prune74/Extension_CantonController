#include "EXCC_Occupation.h"

#include "EXCC_Compteur.h"
#include "EXCC_Ponctuel.h"
#include "EXCC_Booster_WS2812.h"
#include "EXCC_CAN_CC.h"
#include "EXCC_Canton_WS2812.h"
#include "EXCC_Calibration.h"

#include <Arduino.h>

extern EXCC_Canton_WS2812 cantonWS;
extern uint8_t g_idCC;

// ---------------------------------------------------------------------------
//  Variables statiques
// ---------------------------------------------------------------------------
bool EXCC_Occupation::s_occCourant = false;
bool EXCC_Occupation::s_occCompteur = false;
bool EXCC_Occupation::s_occPonctuel = false;

uint8_t EXCC_Occupation::s_voisinsEA = 0;

bool EXCC_Occupation::s_occFinale = false;
bool EXCC_Occupation::s_dernierEnvoi = false;

// ---------------------------------------------------------------------------
//  begin()
// ---------------------------------------------------------------------------
void EXCC_Occupation::begin()
{
    s_occCourant = false;
    s_occCompteur = false;
    s_occPonctuel = false;

    s_voisinsEA = 0;

    s_occFinale = false;
    s_dernierEnvoi = false;
}

// ---------------------------------------------------------------------------
//  majCourant()
// ---------------------------------------------------------------------------
void EXCC_Occupation::majCourant(uint16_t courant_mA)
{
    uint16_t seuilLibre = EXCC_Calibration::getSeuilLibre();
    uint16_t seuilOccupe = EXCC_Calibration::getSeuilOccupe();

    if (s_occCourant)
        s_occCourant = (courant_mA > seuilLibre);
    else
        s_occCourant = (courant_mA > seuilOccupe);
}

// ---------------------------------------------------------------------------
//  majCompteur()
// ---------------------------------------------------------------------------
void EXCC_Occupation::majCompteur(int compteurGlobal)
{
    s_occCompteur = (compteurGlobal > 0);
}

// ---------------------------------------------------------------------------
//  majPonctuels()
// ---------------------------------------------------------------------------
void EXCC_Occupation::majPonctuels(bool actifH, bool actifAH)
{
    s_occPonctuel = (actifH || actifAH);
}

// ---------------------------------------------------------------------------
//  majVoisins()
// ---------------------------------------------------------------------------
void EXCC_Occupation::majVoisins(uint8_t EA)
{
    s_voisinsEA = EA;

    // EA = 0 → canton DOIT être vide
    if (EA == 0)
    {
        EXCC_Compteur::reset();
        s_occCompteur = false;
    }
}

// ---------------------------------------------------------------------------
//  calculerOccupationFinale()
// ---------------------------------------------------------------------------
void EXCC_Occupation::calculerOccupationFinale()
{
    bool occ = false;

    // -----------------------------------------------------------------------
    // 1) Vérification cohérence EA / deltas
    // -----------------------------------------------------------------------
    int dH = EXCC_Compteur::deltaH;
    int dAH = EXCC_Compteur::deltaAH;
    int cg = EXCC_Compteur::compteurGlobal();

    bool incoherent = false;

    switch (s_voisinsEA)
    {
    case 0:
        // Aucun delta autorisé
        if (dH != 0 || dAH != 0 || cg != 0)
            incoherent = true;
        break;

    case 1:
        // SM1 occupé → seul deltaAH autorisé
        if (dH != 0)
            incoherent = true;
        break;

    case 2:
        // SP1 occupé → seul deltaH autorisé
        if (dAH != 0)
            incoherent = true;
        break;

    case 3:
        // Les deux voisins occupés → compteurGlobal doit être > 0
        if (cg == 0)
            incoherent = true;
        break;
    }

    if (incoherent)
    {
        EXCC_Compteur::reset();
        s_occCompteur = false;
        occ = true; // fail-safe
    }

    // -----------------------------------------------------------------------
    // 2) Fusion des sources
    // -----------------------------------------------------------------------
    if (s_occCourant)
        occ = true;
    if (s_occCompteur)
        occ = true;
    if (s_occPonctuel)
        occ = true;

    // EA = 0 → canton DOIT être vide
    if (s_voisinsEA == 0)
        occ = false;

    // -----------------------------------------------------------------------
    // 3) Application si changement
    // -----------------------------------------------------------------------
    if (occ != s_occFinale)
    {
        s_occFinale = occ;
        appliquerChangementOccupation(occ);
    }
}

// ---------------------------------------------------------------------------
//  appliquerChangementOccupation()
// ---------------------------------------------------------------------------
void EXCC_Occupation::appliquerChangementOccupation(bool occupe)
{
    EXCC_CAN_CC::envoyerOccupation(g_idCC, occupe);
    cantonWS.setOccupation(occupe);
    s_dernierEnvoi = occupe;
}

// ---------------------------------------------------------------------------
//  estOccupe()
// ---------------------------------------------------------------------------
bool EXCC_Occupation::estOccupe()
{
    return s_occFinale;
}
