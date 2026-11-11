#include "EXCC_BoosterHardware.h"
#include "EXCC_Pins.h"

#include <Arduino.h>
#include <driver/adc.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

/*
 * ============================================================================
 *  PHASE 1 — version intérimaire :
 *
 *  L'ancien pseudo-DCC (LEDC 20 kHz sur PIN_DCC_PWM + PHASE sur PIN_DRV_PHASE)
 *  est supprimé : ces pins n'existent plus dans le brochage cible.
 *
 *  Version DRV8874 mode PWM (PMODE strapé 3,3 V) :
 *    - applyDcc   : IN1 = bit, IN2 = !bit (digitalWrite intérimaire —
 *                   la Phase 2 remplacera ceci par la régénération RMT
 *                   canal 7 avec les vrais timings 58/100 µs)
 *    - cutout     : IN1 = IN2 = 1 → BRAKE
 *    - failsafe   : IN1 = IN2 = 0 → COAST (Hi-Z), nSLEEP = 0
 *
 *  Tant que rien n'est câblé, ce code compile et s'exécute sans danger.
 * ============================================================================
 */
void EXCC_BoosterHardware::begin()
{
    setupDrv8874();
    setupDccOutputs();
    setupAdc();
}

/* ============================================================
 * setupDrv8874() — DRV8874 mode PWM : IN1/IN2/nSLEEP/FAULT
 * ============================================================ */
void EXCC_BoosterHardware::setupDrv8874()
{
    pinMode(PIN_DRV_IN1, OUTPUT);
    pinMode(PIN_DRV_IN2, OUTPUT);
    pinMode(PIN_DRV_NSLEEP, OUTPUT);
    pinMode(PIN_DRV_FAULT, INPUT_PULLUP);

    // Démarrage propre : COAST puis réveil du driver
    digitalWrite(PIN_DRV_IN1, LOW);
    digitalWrite(PIN_DRV_IN2, LOW);
    digitalWrite(PIN_DRV_NSLEEP, LOW);
    delay(5);
    digitalWrite(PIN_DRV_NSLEEP, HIGH);
}

/* ============================================================
 * setupDccOutputs() — PHASE 1 : remplace l'ancien setupPwmDcc()
 * ============================================================ */
void EXCC_BoosterHardware::setupDccOutputs()
{
    // Rien de plus à configurer en intérim : IN1/IN2 sont en digitalWrite.
    // PHASE 2 : l'init du RMT TX canal 7 (mem_block_num = 1, clk_div 2,
    // ring buffer 4096) + inversion IN2 via matrice GPIO se fera ici,
    // dans EXCC_DccRegen.
}

/* ============================================================
 * applyDcc() — bit DCC reçu du CAN → IN1/IN2 (intérim, sans timing)
 * ============================================================ */
void EXCC_BoosterHardware::applyDcc(const uint8_t *data, uint8_t len)
{
    if (len == 0)
        return;

    m_lastBit = data[0] ? 1 : 0;

    // Intérim : niveau direct sur les entrées du DRV8874.
    // Phase 2 : les bits iront dans le ring buffer RMT (DccRegen::emit)
    // pour une régénération avec les vrais demi-bits 58/100 µs.
    digitalWrite(PIN_DRV_IN1, m_lastBit ? HIGH : LOW);
    digitalWrite(PIN_DRV_IN2, m_lastBit ? LOW : HIGH);   // inversé

    digitalWrite(PIN_DRV_NSLEEP, HIGH);   // sortie active
}

/* ============================================================
 * Cutout RailCom : IN1 = IN2 = 1 → BRAKE (rails au potentiel commun)
 * ============================================================ */
void EXCC_BoosterHardware::enableCutout()
{
    digitalWrite(PIN_DRV_IN1, HIGH);
    digitalWrite(PIN_DRV_IN2, HIGH);
}

void EXCC_BoosterHardware::disableCutout()
{
    // Réinjection du dernier bit connu (reprise biphasée propre)
    digitalWrite(PIN_DRV_IN1, m_lastBit ? HIGH : LOW);
    digitalWrite(PIN_DRV_IN2, m_lastBit ? LOW : HIGH);
}

/* ============================================================
 * enableOutput() / disableOutput()
 * ============================================================ */
void EXCC_BoosterHardware::enableOutput()
{
    digitalWrite(PIN_DRV_NSLEEP, HIGH);
    digitalWrite(PIN_DRV_IN1, m_lastBit ? HIGH : LOW);
    digitalWrite(PIN_DRV_IN2, m_lastBit ? LOW : HIGH);
}

void EXCC_BoosterHardware::disableOutput()
{
    // COAST : IN1 = IN2 = 0 (Hi-Z), driver en veille
    digitalWrite(PIN_DRV_IN1, LOW);
    digitalWrite(PIN_DRV_IN2, LOW);
    digitalWrite(PIN_DRV_NSLEEP, LOW);
}

/* ============================================================
 * isFaultActive()
 * ============================================================ */
bool EXCC_BoosterHardware::isFaultActive()
{
    return digitalRead(PIN_DRV_FAULT) == LOW;
}

/* ============================================================
 * setupAdc() — IPROPI (32/CH4), VRAIL (33/CH5), RailCom HF (36/CH0)
 * ============================================================ */
void EXCC_BoosterHardware::setupAdc()
{
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
}

/* ============================================================
 * readCurrent_mA() — IPROPI : I = V / (R_IPROPI * A_IPROPI)
 * ============================================================ */
uint16_t EXCC_BoosterHardware::readCurrent_mA()
{
    int raw = adc1_get_raw(ADC1_CHANNEL_4);
    float v = (raw / 4095.0f) * 3.3f;
    float currentA = v / (EXCC_IPROPI_R_OHMS * EXCC_IPROPI_GAIN_A_PER_A);
    return (uint16_t)(currentA * 1000.0f);
}

/* ============================================================
 * readVoltage_mV() — pont diviseur R1/R2
 * ============================================================ */
uint16_t EXCC_BoosterHardware::readVoltage_mV()
{
    int raw = adc1_get_raw(ADC1_CHANNEL_5);
    float v = (raw / 4095.0f) * EXCC_ADC_VREF;
    float vrail = v * EXCC_ADC_VOLTAGE_FACTOR;
    return (uint16_t)(vrail * 1000.0f);
}

/* ============================================================
 * readRailcomSample() — intérim : ADC sur GPIO36
 * (Phase 2 : remplacé par le décodage UART2 RX à 250 kbit/s)
 * ============================================================ */
int16_t EXCC_BoosterHardware::readRailcomSample()
{
    return (int16_t)(adc1_get_raw(ADC1_CHANNEL_0) - 2048);
}