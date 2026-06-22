#include "EXCC_Booster_WS2812.h"

#include "EXCC_BoosterHardware.h"
#include "CanDccBooster.h"
#include "EXCC_CanBooster.h"
#include "EXCC_Calibration.h"
#include "EXCC_UartTx.h"
#include "EXCC_Canton_WS2812.h"

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

// ---------------------------------------------------------------------------
//  Instances internes
// ---------------------------------------------------------------------------
//  Le booster matériel (mesure courant/tension, gestion PWM, protections)
static EXCC_BoosterHardware s_hw;

//  Configuration dynamique du booster (courant max, tension min, options)
static BoosterConfig s_cfg;

//  Moteur DCC/Booster (gère le signal DCC, RailCom, télémétrie)
static CanDccBooster s_booster(s_hw, s_cfg);

//  Instance LED du canton (affichage local)
extern EXCC_Canton_WS2812 cantonWS;

// ---------------------------------------------------------------------------
//  Constructeur
// ---------------------------------------------------------------------------
//  Initialise les valeurs internes du booster EXCC.
//  Le booster démarre activé, sans défaut, et avec des mesures nulles.
EXCC_Booster_WS2812::EXCC_Booster_WS2812()
    : m_enabled(true),
      m_current_mA(0),
      m_voltage_mV(0),
      m_faultThermal(false)
{
}

// ---------------------------------------------------------------------------
//  begin()
// ---------------------------------------------------------------------------
//  Initialise le hardware du booster et charge la configuration
//  (courant max, tension min, RailCom, télémétrie).
void EXCC_Booster_WS2812::begin()
{
    s_hw.begin();

    s_cfg.maxCurrent_mA    = EXCC_BOOSTER_MAX_COURANT_mA;
    s_cfg.minVoltage_mV    = EXCC_BOOSTER_MIN_TENSION_mV;
    s_cfg.telemetryEnabled = true;
    s_cfg.railcomEnabled   = true;

    s_booster.setConfig(s_cfg);
}

// ---------------------------------------------------------------------------
//  setEnabled()
// ---------------------------------------------------------------------------
//  Active ou désactive totalement le booster.
//  En cas de désactivation, on coupe la sortie et on remet les mesures à zéro.
void EXCC_Booster_WS2812::setEnabled(bool enabled)
{
    m_enabled = enabled;

    if (!enabled)
    {
        s_hw.disableOutput();

        m_current_mA   = 0;
        m_voltage_mV   = 0;
        m_faultThermal = false;

        // L’occupation n’est plus gérée ici → EXCC_Occupation s’en charge.
        cantonWS.setOccupation(false);
    }
}

// ---------------------------------------------------------------------------
//  update()
// ---------------------------------------------------------------------------
//  Fonction appelée régulièrement (boucle principale).
//  - Met à jour le booster matériel
//  - Récupère la télémétrie (courant, tension, erreurs)
//  - Transmet l’adresse RailCom si détectée
//  - L’occupation n’est plus gérée ici (déplacée dans EXCC_Occupation)
void EXCC_Booster_WS2812::update()
{
    if (!m_enabled)
    {
        s_hw.disableOutput();
        return;
    }

    // Mise à jour du moteur DCC/Booster
    s_booster.update();

    // Lecture de la télémétrie
    const BoosterTelemetry &t = s_booster.getTelemetry();
    m_current_mA   = t.current_mA;
    m_voltage_mV   = t.voltage_mV;
    m_faultThermal = (t.error == BoosterError::HARDWARE_FAULT);

    // Transmission RailCom si une adresse est détectée
    if (t.railcomAddress != BoosterConstants::RAILCOM_NO_ADDRESS)
        EXCC_UartTx::envoyerRailcom(0, t.railcomAddress);

    // L’occupation par courant sera désormais traitée dans EXCC_Occupation
}

// ---------------------------------------------------------------------------
//  onCanMessage()
// ---------------------------------------------------------------------------
//  Réception d’un message CAN destiné au booster (commande DCC, etc.)
void EXCC_Booster_WS2812::onCanMessage(uint32_t id, const uint8_t *data, uint8_t len)
{
    s_booster.onCanMessage(id, data, len);
}

// ---------------------------------------------------------------------------
//  RailCom hooks
// ---------------------------------------------------------------------------
//  Appelés automatiquement lors des fenêtres RailCom
void EXCC_Booster_WS2812::onCutoutStart()
{
    s_booster.onCutoutStart();
}

void EXCC_Booster_WS2812::onCutoutEnd()
{
    s_booster.onCutoutEnd();
}

void EXCC_Booster_WS2812::feedRailcomSample()
{
    s_booster.feedRailcomSample();
}

// ---------------------------------------------------------------------------
//  Accesseurs
// ---------------------------------------------------------------------------
//  Permettent à d’autres modules (EXCC_Occupation, télémétrie, debug)
//  de lire les valeurs mesurées par le booster.
uint16_t EXCC_Booster_WS2812::readCurrent_mA() const { return m_current_mA; }
uint16_t EXCC_Booster_WS2812::readVoltage_mV() const { return m_voltage_mV; }
bool EXCC_Booster_WS2812::isThermalFault() const { return m_faultThermal; }

const BoosterTelemetry &EXCC_Booster_WS2812::getTelemetry() const
{
    return s_booster.getTelemetry();
}
