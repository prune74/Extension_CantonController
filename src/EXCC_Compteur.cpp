#include "EXCC_Compteur.h"

/*
 * ============================================================
 *  Variables statiques
 * ============================================================
 */
uint8_t EXCC_Compteur::s_prevH  = 0;
uint8_t EXCC_Compteur::s_prevAH = 0;

int EXCC_Compteur::s_compteurH  = 0;
int EXCC_Compteur::s_compteurAH = 0;

int EXCC_Compteur::deltaH  = 0;   // <-- AJOUT
int EXCC_Compteur::deltaAH = 0;   // <-- AJOUT

/*
 * ============================================================
 *  begin()
 * ============================================================
 */
void EXCC_Compteur::begin()
{
    s_prevH  = 0;
    s_prevAH = 0;

    s_compteurH  = 0;
    s_compteurAH = 0;

    deltaH  = 0;
    deltaAH = 0;
}

/*
 * ============================================================
 *  reset()
 * ============================================================
 */
void EXCC_Compteur::reset()
{
    s_compteurH  = 0;
    s_compteurAH = 0;

    deltaH  = 0;
    deltaAH = 0;
}

/*
 * ============================================================
 *  loop()
 * ============================================================
 */
void EXCC_Compteur::loop()
{
    // Reset des deltas pour cette frame
    deltaH  = 0;
    deltaAH = 0;

    traiterQuadratureH();
    traiterQuadratureAH();
}

/*
 * ============================================================
 *  traiterQuadratureH()
 * ============================================================
 */
void EXCC_Compteur::traiterQuadratureH()
{
    uint8_t etat;

    while (quadH.lireEvenement(etat))
    {
        int delta = deltaFromQuadrature(s_prevH, etat);
        s_prevH = etat;

        deltaH += delta;   // <-- AJOUT

        s_compteurH += delta;
        if (s_compteurH < 0)
            s_compteurH = 0;
    }
}

/*
 * ============================================================
 *  traiterQuadratureAH()
 * ============================================================
 */
void EXCC_Compteur::traiterQuadratureAH()
{
    uint8_t etat;

    while (quadAH.lireEvenement(etat))
    {
        int delta = deltaFromQuadrature(s_prevAH, etat);
        s_prevAH = etat;

        deltaAH += delta;  // <-- AJOUT

        s_compteurAH += delta;
        if (s_compteurAH < 0)
            s_compteurAH = 0;
    }
}

/*
 * ============================================================
 *  deltaFromQuadrature()
 * ------------------------------------------------------------
 *  Table quadrature standard :
 *
 *      prev → curr : delta
 *      00 → 01 : +1
 *      01 → 11 : +1
 *      11 → 10 : +1
 *      10 → 00 : +1
 *
 *      00 → 10 : -1
 *      10 → 11 : -1
 *      11 → 01 : -1
 *      01 → 00 : -1
 * ============================================================
 */
int EXCC_Compteur::deltaFromQuadrature(uint8_t prev, uint8_t curr)
{
    static const int8_t table[4][4] = {
        // curr: 00  01  10  11
        /* prev=00 */ { 0, +1, -1,  0 },
        /* prev=01 */ { -1, 0,  0, +1 },
        /* prev=10 */ { +1, 0,  0, -1 },
        /* prev=11 */ { 0, -1, +1,  0 }
    };

    return table[prev][curr];
}

/*
 * ============================================================
 *  Accesseurs
 * ============================================================
 */
int EXCC_Compteur::compteurH()      { return s_compteurH; }
int EXCC_Compteur::compteurAH()     { return s_compteurAH; }
int EXCC_Compteur::compteurGlobal() { return s_compteurH + s_compteurAH; }
