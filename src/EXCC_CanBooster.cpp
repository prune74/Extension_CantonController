#include "EXCC_CanBooster.h"
#include "EXCC_CanConfig.h"
#include "CanInit.h"
#include "CanBus.h"
#include "EXCC_Booster_WS2812.h"

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

// Instance globale du booster
extern EXCC_Booster_WS2812 booster;

/*
 * ============================================================================
 *  SECTION : Configuration CAN
 * ============================================================================
 */
static EXCC_CanConfig canConfig;

/*
 * ============================================================================
 *  SECTION : État interne RailCom
 * ============================================================================
 */
static bool s_cutoutActive = false;

/*
 * ============================================================================
 *  SECTION : Timestamp activité CAN
 * ============================================================================
 */
static uint32_t s_lastCanRxMs = 0;

/*
 * ============================================================================
 *  SECTION : Timestamp cutout global
 * ============================================================================
 *  Mis à jour à chaque trame 0x101 (début ou fin de cutout).
 *  Utilisé pour la LED RailCom (état global du cutout).
 */
static uint32_t s_lastCutoutMs = 0;

/*
 * ============================================================================
 *  EXCC_CanBooster::begin()
 * ============================================================================
 */
void EXCC_CanBooster::begin()
{
    CanInit::begin(canConfig);
}

/*
 * ============================================================================
 *  EXCC_CanBooster::process()
 * ============================================================================
 */
void EXCC_CanBooster::process()
{
    CanMsg msg;

    while (CanBus::bus(0).receive(msg))
    {
        s_lastCanRxMs = millis();   // Activité CAN détectée
        handleFrame(msg);
    }
}

/*
 * ============================================================================
 *  EXCC_CanBooster::handleFrame()
 * ============================================================================
 */
void EXCC_CanBooster::handleFrame(const CanMsg &msg)
{
    // ------------------------------------------------------------------------
    // Détection du cutout RailCom (ID 0x101)
    // ------------------------------------------------------------------------
    if (msg.id == 0x101 && msg.dlc >= 1)
    {
        bool active = (msg.data[0] != 0);

        // Mise à jour du timestamp cutout global
        s_lastCutoutMs = millis();

        // Début du cutout
        if (active && !s_cutoutActive)
        {
            s_cutoutActive = true;
            booster.onCutoutStart();
        }

        // Fin du cutout
        else if (!active && s_cutoutActive)
        {
            s_cutoutActive = false;
            booster.onCutoutEnd();
        }
    }

    // ------------------------------------------------------------------------
    // Transmission de la trame au booster
    // ------------------------------------------------------------------------
    booster.onCanMessage(msg.id, msg.data, msg.dlc);
}

/*
 * ============================================================================
 *  EXCC_CanBooster::isCutoutActive()
 * ============================================================================
 */
bool EXCC_CanBooster::isCutoutActive()
{
    return s_cutoutActive;
}

/*
 * ============================================================================
 *  EXCC_CanBooster::lastCanRxMs()
 * ============================================================================
 */
uint32_t EXCC_CanBooster::lastCanRxMs()
{
    return s_lastCanRxMs;
}

/*
 * ============================================================================
 *  EXCC_CanBooster::lastCutoutMs()
 * ============================================================================
 *  Utilisé pour la LED RailCom (état global du cutout).
 */
uint32_t EXCC_CanBooster::lastCutoutMs()
{
    return s_lastCutoutMs;
}
