#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "EXCC_Pins.h"

/* ============================================================
 * EXCC_RailCom — adresse RailCom pendant le cutout
 *
 * Front-end carte : pont BAT54 + ampli TLV9151 + opto TLP2362
 * → RAILCOM_ISO : signal LOGIQUE 3,3 V isolé (GPIO36).
 * Décodage UART2 250 kbit/s (NMRA S-9.3.2), zéro jigue CPU.
 *
 * EXCC : modèle NON BLOQUANT en deux temps (le cutout est piloté
 * par les trames 0x101 START/END du convertisseur, déjà asynchrones) :
 *   arm()    → début de cutout : purge le RX
 *   collect()→ fin de cutout : lit les octets arrivés pendant la
 *              fenêtre et décode l'adresse (0 = rien)
 * ============================================================ */
class EXCC_RailCom
{
public:
    bool begin();
    void arm();                 // appelé sur 0x101 START (cutout BRAKE)
    uint16_t collect();         // appelé sur 0x101 END → adresse ou 0

    // Statistiques (diagnostic)
    uint32_t frames = 0;
    uint32_t valid = 0;

private:
    bool ready = false;
    static constexpr uint8_t MAX_BYTES = 4; // jusqu'à 2 paquets par cutout
    static uint16_t decode(uint8_t ch1, uint8_t ch2);
};