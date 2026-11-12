#include "EXCC_BoosterHardware.h"
#include "EXCC_Pins.h"

#include <Arduino.h>
#include <driver/adc.h>
#include "soc/gpio_sig_map.h" // RMT_SIG_OUT0_IDX

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

// 0x100 = SIG_GPIO_OUT : déconnecte la pin du périphérique (matrice)
#define GPIO_SIG_DETACH 0x100

uint32_t EXCC_BoosterHardware::s_bitsEmis = 0;

void EXCC_BoosterHardware::begin()
{
    setupDrv8874();
    setupAdc();

    // Régénération RMT canal 7 — LE point à risque de la Phase 2
    const bool ok = m_regen.begin();
#if EXCC_DEBUG
    Serial.printf("[BOOSTER] DccRegen RMT ch7 : %s\n", ok ? "OK" : "ERREUR");
#endif

    connectDcc();
}

/* ============================================================
 * setupDrv8874() — nSLEEP (latch PMODE), FAULT, état initial COAST
 * ============================================================ */
void EXCC_BoosterHardware::setupDrv8874()
{
    pinMode(PIN_DRV_FAULT, INPUT_PULLUP);
    pinMode(PIN_DRV_NSLEEP, OUTPUT);
    digitalWrite(PIN_DRV_NSLEEP, LOW); // COAST d'abord
    delay(5);
    digitalWrite(PIN_DRV_NSLEEP, HIGH); // lève nSLEEP : PMODE (PWM) latché
}

/* ============================================================
 * connectDcc() — IN1 = RMT direct, IN2 = même signal inversé
 * ============================================================ */
void EXCC_BoosterHardware::connectDcc()
{
    gpio_matrix_out((gpio_num_t)PIN_DRV_IN1, RMT_SIG_OUT0_IDX + RMT_DCC_CHANNEL, false, false);
    gpio_matrix_out((gpio_num_t)PIN_DRV_IN2, RMT_SIG_OUT0_IDX + RMT_DCC_CHANNEL, true, false);
    m_connected = true;
    m_braking = false;
}

/* ============================================================
 * applyDcc() — bit DCC du CAN → item RMT (timings matériels)
 * ============================================================ */
void EXCC_BoosterHardware::applyDcc(const uint8_t *data, uint8_t len)
{
    if (len == 0)
        return;

    const uint8_t bit = data[0] ? 1 : 0;
    if (m_regen.emit(&bit, 1) != 1)
        return;

    digitalWrite(PIN_DRV_NSLEEP, HIGH); // sortie active
    s_bitsEmis++;
}

/* ============================================================
 * Cutout : détache les IN du RMT, force 1/1 → BRAKE
 * ============================================================ */
void EXCC_BoosterHardware::enableCutout()
{
    gpio_matrix_out((gpio_num_t)PIN_DRV_IN1, GPIO_SIG_DETACH, false, false);
    gpio_matrix_out((gpio_num_t)PIN_DRV_IN2, GPIO_SIG_DETACH, false, false);
    gpio_set_level((gpio_num_t)PIN_DRV_IN1, 1);
    gpio_set_level((gpio_num_t)PIN_DRV_IN2, 1);
    m_braking = true;
}

void EXCC_BoosterHardware::disableCutout()
{
    connectDcc(); // rebranche le biphasé
}

/* ============================================================
 * enableOutput() / disableOutput() — reconnexion / COAST
 * ============================================================ */
void EXCC_BoosterHardware::enableOutput()
{
    digitalWrite(PIN_DRV_NSLEEP, HIGH);
    if (!m_connected)
        connectDcc();
}

void EXCC_BoosterHardware::disableOutput()
{
    gpio_matrix_out((gpio_num_t)PIN_DRV_IN1, GPIO_SIG_DETACH, false, false);
    gpio_matrix_out((gpio_num_t)PIN_DRV_IN2, GPIO_SIG_DETACH, false, false);
    gpio_set_level((gpio_num_t)PIN_DRV_IN1, 0);
    gpio_set_level((gpio_num_t)PIN_DRV_IN2, 0);
    digitalWrite(PIN_DRV_NSLEEP, LOW);
    m_connected = false;
    m_braking = false;
}

bool EXCC_BoosterHardware::isFaultActive()
{
    return digitalRead(PIN_DRV_FAULT) == LOW;
}

/* ============================================================
 * ADC — IPROPI (32/CH4), VRAIL (33/CH5)
 * ============================================================ */
void EXCC_BoosterHardware::setupAdc()
{
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);
}

uint16_t EXCC_BoosterHardware::readCurrent_mA()
{
    int raw = adc1_get_raw(ADC1_CHANNEL_4);
    float v = (raw / 4095.0f) * 3.3f;
    float currentA = v / (EXCC_IPROPI_R_OHMS * EXCC_IPROPI_GAIN_A_PER_A);
    return (uint16_t)(currentA * 1000.0f);
}

uint16_t EXCC_BoosterHardware::readVoltage_mV()
{
    int raw = adc1_get_raw(ADC1_CHANNEL_5);
    float v = (raw / 4095.0f) * EXCC_ADC_VREF;
    float vrail = v * EXCC_ADC_VOLTAGE_FACTOR;
    return (uint16_t)(vrail * 1000.0f);
}

/* Obsolète Phase 2 : l'UART2 a remplacé l'ADC HF. */
int16_t EXCC_BoosterHardware::readRailcomSample()
{
    return 0;
}