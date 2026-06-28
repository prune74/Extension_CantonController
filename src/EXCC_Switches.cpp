/*
 * Module : EXCC_Switches
 * Rôle   : Lecture des micro‑switchs d’aiguilles + sécurité anti‑blocage.
 *
 * Fonctionnement général :
 *   - Chaque aiguille possède 2 micro‑switchs :
 *        • DROIT
 *        • DEVIÉ
 *
 *   - Le module déduit :
 *        • la position réelle : DROIT / DEVIÉ / INDET / INCOHÉRENT
 *        • l’état sécurité    : OK / BLOQUÉ / ERREUR
 *
 *   - Le module envoie au CC :
 *        • trame 0x06 (position + état)
 *          → sur changement immédiat
 *          → périodiquement (200 ms)
 *
 *   - Le module surveille les mouvements :
 *        • notifierMouvementDemarre() appelé par EXCC_Servo
 *        • calcul dynamique du temps max autorisé
 *        • détection blocage si dépassement
 *
 * Contraintes :
 *   - Aucun malloc
 *   - Lecture MCP23017 non bloquante
 *   - update() doit être très court (appelé ~1 kHz)
 *
 * Notes :
 *   - La sécurité est entièrement locale : même si le CC plante,
 *     EXCC_Switches continue de surveiller les aiguilles.
 */

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include "EXCC_Switches.h"
#include "EXCC_Pins.h"
#include "EXCC_Config.h"
#include "EXCC_CAN_CC.h"
#include "EXCC_Servo.h"
#include "Protocol.h"
#include "Protocol.h"
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

/*
 * ============================================================
 *  Variables statiques
 * ------------------------------------------------------------
 *  Ces tableaux mémorisent :
 *    - les derniers états envoyés au CC
 *    - les timers d’envoi périodique
 *    - les timers de détection de blocage
 *
 *  Ils sont définis ici (et non dans le header) car ils ne
 *  doivent exister qu’une seule fois dans tout le programme.
 * ============================================================
 */

Adafruit_MCP23X17 EXCC_Switches::mcp;

extern uint8_t g_idCC;

/*
 * Définition des tableaux constexpr déclarés dans le .h
 * ------------------------------------------------------------
 * ⚠️ Obligatoire sur ESP32 / GCC 8.4.0 :
 *    Un tableau static constexpr DOIT avoir une définition
 *    dans un .cpp, sinon → undefined reference.
 */
constexpr uint8_t EXCC_Switches::swDroit[AIG_COUNT] = {
    MCP_SW_H_1_DROIT,  // Aiguille 1 → Horaire
    MCP_SW_H_2_DROIT,  // Aiguille 2 → Horaire
    MCP_SW_H_3_DROIT,  // Aiguille 3 → Horaire
    MCP_SW_AH_1_DROIT, // Aiguille 4 → Anti‑Horaire
    MCP_SW_AH_2_DROIT, // Aiguille 5 → Anti‑Horaire
    MCP_SW_AH_3_DROIT  // Aiguille 6 → Anti‑Horaire
};

constexpr uint8_t EXCC_Switches::swDevie[AIG_COUNT] = {
    MCP_SW_H_1_DEVIE,  // Aiguille 0 → Horaire
    MCP_SW_H_2_DEVIE,  // Aiguille 1 → Horaire
    MCP_SW_H_3_DEVIE,  // Aiguille 2 → Horaire
    MCP_SW_AH_1_DEVIE, // Aiguille 3 → Anti‑Horaire
    MCP_SW_AH_2_DEVIE, // Aiguille 4 → Anti‑Horaire
    MCP_SW_AH_3_DEVIE  // Aiguille 5 → Anti‑Horaire
};

// Derniers états envoyés au CC
uint8_t EXCC_Switches::lastPos[AIG_COUNT] = {255, 255, 255};
uint8_t EXCC_Switches::lastEtat[AIG_COUNT] = {255, 255, 255};
uint32_t EXCC_Switches::lastSendMs[AIG_COUNT] = {0, 0, 0};

// Suivi du mouvement
uint32_t EXCC_Switches::moveStartMs[AIG_COUNT] = {0, 0, 0};
uint32_t EXCC_Switches::moveMaxMs[AIG_COUNT] = {0, 0, 0};
bool EXCC_Switches::movementActive[AIG_COUNT] = {false, false, false};

/*
 * ============================================================
 *  Helper : calcul du temps max de mouvement
 * ------------------------------------------------------------
 *  Le temps max dépend :
 *    - de l’amplitude réelle du mouvement (PWM)
 *    - de la vitesse configurée
 *
 *  On ajoute une marge de sécurité.
 * ============================================================
 */
static uint32_t calculerTempsMaxMouvement(uint8_t idx)
{
    uint16_t cur = EXCC_Servo::getCurrentPwm(idx);
    uint16_t tgt = EXCC_Servo::getTargetPwm(idx);

    if (cur == 0 || tgt == 0)
        return 800;

    uint16_t amplitude = (cur > tgt) ? (cur - tgt) : (tgt - cur);

    uint32_t temps_theorique = amplitude / 2;

    if (temps_theorique < 200)
        temps_theorique = 200;
    if (temps_theorique > 1500)
        temps_theorique = 1500;

    return temps_theorique + 200;
}

/*
 * ============================================================
 *  begin()
 * ------------------------------------------------------------
 *  Configure les entrées MCP23017 pour les micro‑switchs.
 * ============================================================
 */
void EXCC_Switches::begin()
{
    mcp.begin_I2C(MCP23017_ADDR);

    for (uint8_t i = 0; i < AIG_COUNT; i++)
    {
        mcp.pinMode(swDroit[i], INPUT_PULLUP);
        mcp.pinMode(swDevie[i], INPUT_PULLUP);
    }
}

/*
 * ============================================================
 *  notifierMouvementDemarre()
 * ------------------------------------------------------------
 *  Appelé par EXCC_Servo lorsqu’un mouvement F0 commence.
 *  Initialise le timer de surveillance anti‑blocage.
 * ============================================================
 */
void EXCC_Switches::notifierMouvementDemarre(uint8_t idx)
{
    if (idx >= AIG_COUNT)
        return;

    movementActive[idx] = true;
    moveStartMs[idx] = millis();
    moveMaxMs[idx] = calculerTempsMaxMouvement(idx);
}

/*
 * ============================================================
 *  lirePosition()
 * ============================================================
 */
uint8_t EXCC_Switches::lirePosition(uint8_t idx)
{
    bool droit = !mcp.digitalRead(swDroit[idx]);
    bool devie = !mcp.digitalRead(swDevie[idx]);

    if (droit && !devie)
        return static_cast<uint8_t>(ExccCode::POS_DROIT);
    if (!droit && devie)
        return static_cast<uint8_t>(ExccCode::POS_DEVIE);
    if (!droit && !devie)
        return static_cast<uint8_t>(ExccCode::POS_INDET);

    return static_cast<uint8_t>(ExccCode::POS_INCOHERENT);
}

/*
 * ============================================================
 *  lireEtat()
 * ============================================================
 */
uint8_t EXCC_Switches::lireEtat(uint8_t idx, uint8_t pos)
{
    if (pos == static_cast<uint8_t>(ExccCode::POS_INCOHERENT))
        return static_cast<uint8_t>(ExccCode::ETAT_ERREUR);

    if (pos == static_cast<uint8_t>(ExccCode::POS_INDET))
        return static_cast<uint8_t>(ExccCode::ETAT_ERREUR);

    if (movementActive[idx])
    {
        uint32_t now = millis();
        if (now - moveStartMs[idx] > moveMaxMs[idx])
            return static_cast<uint8_t>(ExccCode::ETAT_BLOQUE);
    }

    return static_cast<uint8_t>(ExccCode::ETAT_OK);
}

/*
 * ============================================================
 *  envoyerTrame()
 * ============================================================
 */
void EXCC_Switches::envoyerTrame(uint8_t idx, uint8_t pos, uint8_t etat)
{
    EXCC_CAN_CC::envoyerPositionAiguille(g_idCC, idx, pos, etat);
    lastSendMs[idx] = millis();
}

/*
 * ============================================================
 *  update()
 * ============================================================
 */
void EXCC_Switches::update()
{
    for (uint8_t i = 0; i < AIG_COUNT; i++)
    {
        uint8_t pos = lirePosition(i);
        uint8_t etat = lireEtat(i, pos);

        // Fin de mouvement si position stable et OK
        if ((pos == static_cast<uint8_t>(ExccCode::POS_DROIT) || 
             pos == static_cast<uint8_t>(ExccCode::POS_DEVIE)) &&
            etat == static_cast<uint8_t>(ExccCode::ETAT_OK))
        {
            movementActive[i] = false;
        }

        bool changed = (pos != lastPos[i]) || (etat != lastEtat[i]);
        bool periodic = (millis() - lastSendMs[i] >= 200);

        if (changed || periodic)
        {
            envoyerTrame(i, pos, etat);
            lastPos[i] = pos;
            lastEtat[i] = etat;
        }
    }
}

/*
 * ============================================================
 *  getWorstState()
 *  → utilisé par EXCC_StatusLed (LED Switches 4D)
 * ============================================================
 */
uint8_t EXCC_Switches::getWorstState()
{
    uint8_t worst = static_cast<uint8_t>(ExccCode::ETAT_OK);

    for (uint8_t i = 0; i < AIG_COUNT; i++)
    {
        uint8_t e = lastEtat[i];

        if (e == static_cast<uint8_t>(ExccCode::ETAT_BLOQUE))
            return static_cast<uint8_t>(ExccCode::ETAT_BLOQUE); // priorité max

        if (e == static_cast<uint8_t>(ExccCode::ETAT_ERREUR))
            worst = static_cast<uint8_t>(ExccCode::ETAT_ERREUR);
    }

    return worst;
}

/*
 * ============================================================
 *  anyMovement()
 *  → indique si au moins une aiguille bouge
 * ============================================================
 */
bool EXCC_Switches::anyMovement()
{
    for (uint8_t i = 0; i < AIG_COUNT; i++)
    {
        if (movementActive[i])
            return true;
    }
    return false;
}
