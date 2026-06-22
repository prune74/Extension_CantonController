#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "EXCC_Config.h"

class BoosterTelemetry;   // Forward declaration

/*
 * ============================================================================
 *  EXCC_Booster_WS2812
 *  --------------------------------------------------------------------------
 *  Cette classe encapsule la gestion du booster DCC côté EXCC :
 *
 *   - Gestion du hardware booster (courant, tension, protections)
 *   - Gestion du signal DCC (via CanDccBooster)
 *   - Gestion RailCom (cutout, échantillonnage, adresse)
 *   - Mise à disposition de la télémétrie pour les autres modules
 *
 *  IMPORTANT :
 *    → L’occupation n’est plus gérée ici.
 *      Elle est désormais centralisée dans EXCC_Occupation.
 *
 *  Le booster se contente de mesurer, transmettre et protéger.
 * ============================================================================
 */
class EXCC_Booster_WS2812
{
public:
    EXCC_Booster_WS2812();

    // Initialisation du hardware et de la configuration du booster
    void begin();

    // Mise à jour périodique (lecture télémétrie, RailCom, protections)
    void update();

    // Activation / désactivation complète du booster
    void setEnabled(bool enabled);

    // Messages CAN destinés au booster (commandes DCC)
    void onCanMessage(uint32_t id, const uint8_t *data, uint8_t len);

    // Hooks RailCom (fenêtre cutout)
    void onCutoutStart();
    void onCutoutEnd();
    void feedRailcomSample();

    // Accès aux mesures
    uint16_t readCurrent_mA() const;
    uint16_t readVoltage_mV() const;
    bool isThermalFault() const;

    // Accès direct à la télémétrie brute
    const BoosterTelemetry& getTelemetry() const;

    // Utilisé par EXCC_StatusLed
    bool isEnabled() const { return m_enabled; }

private:
    // Ancienne gestion d’occupation → supprimée
    void updateOccupation(uint16_t courant_mA);   // <<< sera supprimée définitivement
    //void updateLedState();                        // <<< idem (migration LED 1)

private:
    bool m_enabled;        // Booster activé / désactivé

    uint16_t m_current_mA; // Courant mesuré
    uint16_t m_voltage_mV; // Tension mesurée
    bool m_faultThermal;   // Défaut thermique détecté
};
