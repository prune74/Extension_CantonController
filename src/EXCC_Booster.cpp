#include "EXCC_Booster.h"

#include "EXCC_Config.h"
#include "EXCC_UartTx.h"
#include "EXCC_Canton_WS2812.h"
#include "EXCC_Calibration.h"

#include "EXCC_BoosterHardware.h"
#include "CanDccBooster.h"

#include <Arduino.h>

// ============================================================================
//  Architecture Booster EXCC (2026)
// ============================================================================
//
//  Le booster repose sur trois couches logicielles :
//
//   1) EXCC_BoosterHardware
//        → Accès direct au matériel (MOSFET, ADC, protections)
//
//   2) BoosterConfig
//        → Paramètres dynamiques (seuils, RailCom, télémétrie)
//
//   3) CanDccBooster
//        → Cœur logique :
//             - application du DCC
//             - gestion du cutout RailCom
//             - sécurité (courant, tension, fautes)
//             - décodage RailCom HF
//             - mise à jour de la télémétrie
//
//  Ces objets sont statiques car un seul booster existe sur EXCC.
// ============================================================================

static EXCC_BoosterHardware s_hw;
static BoosterConfig s_cfg;
static CanDccBooster s_booster(s_hw, s_cfg);

// État interne du booster
static bool s_enabled = true;
static bool s_lastOccupe = false;

// Télémétrie locale
static uint16_t s_current_mA = 0;
static uint16_t s_voltage_mV = 0;
static bool s_faultThermal = false;

// Canton WS2812 (déclaré dans EXCC_Main.cpp)
extern EXCC_Canton_WS2812 cantonWS;

// ============================================================================
//  EXCC_Booster::begin()
// ----------------------------------------------------------------------------
//  Initialise le matériel et configure la bibliothèque CanDccBooster.
// ============================================================================
void EXCC_Booster::begin()
{
    // Initialisation hardware (GPIO, ADC, protections…)
    s_hw.begin();

    // Configuration initiale (définie dans EXCC_Config.h)
    s_cfg.maxCurrent_mA   = EXCC_BOOSTER_MAX_COURANT_mA;
    s_cfg.minVoltage_mV   = EXCC_BOOSTER_MIN_TENSION_mV;
    s_cfg.telemetryEnabled = true;
    s_cfg.railcomEnabled   = true;

    // Transmission de la configuration au cœur logique
    s_booster.setConfig(s_cfg);
}

// ============================================================================
//  EXCC_Booster::setEnabled()
// ----------------------------------------------------------------------------
//  Active ou désactive complètement la sortie voie.
// ============================================================================
void EXCC_Booster::setEnabled(bool enabled)
{
    s_enabled = enabled;

    if (!enabled)
    {
        // Coupure matérielle immédiate
        s_hw.disableOutput();

        // Reset télémétrie locale
        s_current_mA = 0;
        s_voltage_mV = 0;
        s_faultThermal = false;
        s_lastOccupe = false;

        // Notification canton (WS2812)
        cantonWS.setOccupation(false);
    }
}

// ============================================================================
//  EXCC_Booster::update()
// ----------------------------------------------------------------------------
//  Appelé toutes les 1 ms par EXCC_BoosterCore.
//
//  Rôle :
//    - exécuter la logique DCC + RailCom
//    - récupérer la télémétrie
//    - transmettre l’adresse RailCom détectée
//    - mettre à jour l’occupation du canton
// ============================================================================
void EXCC_Booster::update()
{
    if (!s_enabled)
    {
        s_hw.disableOutput();
        return;
    }

    // Mise à jour du cœur logique
    s_booster.update();

    // Lecture télémétrie
    const BoosterTelemetry &t = s_booster.getTelemetry();
    s_current_mA   = t.current_mA;
    s_voltage_mV   = t.voltage_mV;
    s_faultThermal = (t.error == BoosterError::HARDWARE_FAULT);

    // ------------------------------------------------------------------------
    //  RailCom : adresse détectée
    // ------------------------------------------------------------------------
    if (t.railcomAddress != BoosterConstants::RAILCOM_NO_ADDRESS)
    {
        // 0 = numéro de canton local sur cette carte EXCC
        EXCC_UartTx::envoyerRailcom(0, t.railcomAddress);
    }

    // Mise à jour occupation canton
    updateOccupation(s_current_mA);
}

// ============================================================================
//  EXCC_Booster::onCanMessage()
// ----------------------------------------------------------------------------
//  Réception d’une trame CAN Booster.
// ============================================================================
void EXCC_Booster::onCanMessage(uint32_t id, const uint8_t *data, uint8_t len)
{
    s_booster.onCanMessage(id, data, len);
}

// ============================================================================
//  EXCC_Booster::updateOccupation()
// ----------------------------------------------------------------------------
//  Détection de l’occupation via le courant consommé.
//  Utilise les seuils calibrés + hystérésis.
// ============================================================================
void EXCC_Booster::updateOccupation(uint16_t courant_mA)
{
    uint16_t seuilLibre  = EXCC_Calibration::getSeuilLibre();
    uint16_t seuilOccupe = EXCC_Calibration::getSeuilOccupe();

    bool occupe;

    // Hystérésis
    if (s_lastOccupe)
        occupe = (courant_mA > seuilLibre);
    else
        occupe = (courant_mA > seuilOccupe);

    // Changement d’état → notification UART + LED canton
    if (occupe != s_lastOccupe)
    {
        s_lastOccupe = occupe;

        EXCC_UartTx::envoyerOccupation(occupe);
        cantonWS.setOccupation(occupe);
    }
}

// ============================================================================
//  Hooks RailCom (appelés par EXCC_CanBooster)
// ============================================================================
void EXCC_Booster::onCutoutStart() { s_booster.onCutoutStart(); }
void EXCC_Booster::onCutoutEnd()   { s_booster.onCutoutEnd(); }
void EXCC_Booster::feedRailcomSample() { s_booster.feedRailcomSample(); }

// ============================================================================
//  Accesseurs publics
// ============================================================================
uint16_t EXCC_Booster::readCurrent_mA() { return s_current_mA; }
uint16_t EXCC_Booster::readVoltage_mV() { return s_voltage_mV; }
bool EXCC_Booster::isThermalFault()     { return s_faultThermal; }
