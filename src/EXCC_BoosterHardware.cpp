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

static constexpr int LEDC_CHANNEL = 0;

/*
 * ============================================================================
 *  EXCC_BoosterHardware::begin()
 * ============================================================================
 *  Point d’entrée du module matériel du booster.
 *
 *  Le booster repose sur trois sous-systèmes :
 *    1) DRV8874 → pont en H qui génère le DCC
 *    2) PWM LEDC → modulation du signal DCC
 *    3) ADC → mesures courant / tension / RailCom HF
 *
 *  ⚠️ IMPORTANT :
 *    Le RailCom HF est lu via l’ADC interne de l’ESP32 (ADC1_CHANNEL_0).
 *    Ce GPIO (GPIO36) n’apparaît PAS dans EXCC_Pins.h car :
 *      - c’est une entrée ADC ONLY
 *      - elle ne supporte pas pinMode()
 *      - elle ne supporte pas digitalRead()
 *      - elle est réservée à l’ADC interne
 */
void EXCC_BoosterHardware::begin()
{
    setupDrv8874();
    setupPwmDcc();
    setupAdc();   // <-- Configure aussi l’entrée RailCom HF
}

/*
 * ============================================================================
 *  setupDrv8874()
 * ============================================================================
 *  Configure le driver DRV8874 :
 *    - nSLEEP : active/désactive le pont en H
 *    - PHASE  : polarité du signal DCC
 *    - FAULT  : détection des erreurs matérielles
 */
void EXCC_BoosterHardware::setupDrv8874()
{
    pinMode(PIN_DRV_NSLEEP, OUTPUT);
    pinMode(PIN_DRV_PHASE, OUTPUT);
    pinMode(PIN_DRV_FAULT, INPUT_PULLUP);

    // Démarrage propre
    digitalWrite(PIN_DRV_NSLEEP, LOW);
    digitalWrite(PIN_DRV_PHASE, LOW);
    delay(5);

    // Activation du driver
    digitalWrite(PIN_DRV_NSLEEP, HIGH);
}

/*
 * ============================================================================
 *  enableOutput() / disableOutput()
 * ============================================================================
 *  Active ou coupe totalement la sortie DCC.
 */
void EXCC_BoosterHardware::enableOutput()
{
    ledcWrite(LEDC_CHANNEL, 255);
    digitalWrite(PIN_DRV_NSLEEP, HIGH);
}

void EXCC_BoosterHardware::disableOutput()
{
    ledcWrite(LEDC_CHANNEL, 0);
    digitalWrite(PIN_DRV_NSLEEP, LOW);
}

/*
 * ============================================================================
 *  isFaultActive()
 * ============================================================================
 *  Retourne true si le DRV8874 signale une erreur matérielle.
 */
bool EXCC_BoosterHardware::isFaultActive()
{
    return digitalRead(PIN_DRV_FAULT) == LOW;
}

/*
 * ============================================================================
 *  setupPwmDcc()
 * ============================================================================
 *  Configure le PWM matériel pour générer le DCC.
 */
void EXCC_BoosterHardware::setupPwmDcc()
{
    ledcSetup(LEDC_CHANNEL, 20000, 8);
    ledcAttachPin(PIN_DCC_PWM, LEDC_CHANNEL);
    ledcWrite(LEDC_CHANNEL, 0);
}

/*
 * ============================================================================
 *  applyDcc()
 * ============================================================================
 *  Applique un bit DCC reçu via le CAN Booster.
 */
void EXCC_BoosterHardware::applyDcc(const uint8_t *data, uint8_t len)
{
    if (len == 0)
        return;

    digitalWrite(PIN_DRV_PHASE, data[0] ? HIGH : LOW);
    enableOutput();
}

/*
 * ============================================================================
 *  enableCutout() / disableCutout()
 * ============================================================================
 *  Le cutout RailCom coupe brièvement le DCC pour permettre
 *  aux décodeurs de parler.
 *
 *  → Pendant cette fenêtre, EXCC lit le RailCom HF via l’ADC.
 */
void EXCC_BoosterHardware::enableCutout()
{
    ledcWrite(LEDC_CHANNEL, 0);
}

void EXCC_BoosterHardware::disableCutout()
{
    ledcWrite(LEDC_CHANNEL, 255);
}

/*
 * ============================================================================
 *  setupAdc()
 * ============================================================================
 *  Configure les ADC internes :
 *    - ADC1_CHANNEL_4 → courant (GPIO32)
 *    - ADC1_CHANNEL_5 → tension (GPIO33)
 *    - ADC1_CHANNEL_0 → RailCom HF (GPIO36)
 */
void EXCC_BoosterHardware::setupAdc()
{
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
}

/*
 * ============================================================================
 *  readCurrent_mA()
 * ============================================================================
 *  Mesure le courant via IPROPI.
 */
uint16_t EXCC_BoosterHardware::readCurrent_mA()
{
    // Lecture brute ADC1 (GPIO32 → ADC1_CHANNEL_4)
    int raw = adc1_get_raw(ADC1_CHANNEL_4);

    // Conversion en tension
    float v = (raw / 4095.0f) * 3.3f;

    // Conversion en courant (A)
    float currentA = v / (EXCC_IPROPI_R_OHMS * EXCC_IPROPI_GAIN_A_PER_A);
    return (uint16_t)(currentA * 1000.0f);
}

/*
 * ============================================================================
 *  readVoltage_mV()
 * ============================================================================
 *  Mesure la tension voie DCC.
 */
uint16_t EXCC_BoosterHardware::readVoltage_mV()
{
    // Lecture brute ADC1 (GPIO33 → ADC1_CHANNEL_5)
    int raw = adc1_get_raw(ADC1_CHANNEL_5);

    // Conversion en tension ADC
    float v = (raw / 4095.0f) * EXCC_ADC_VREF;

    // Application du diviseur de tension
    float vrail = v * EXCC_ADC_VOLTAGE_FACTOR;
    return (uint16_t)(vrail * 1000.0f);
}

/*
 * ============================================================================
 *  readRailcomSample()
 * ============================================================================
 *  Retourne un échantillon brut du signal RailCom HF.
 *
 *  ⚠️ C’est la fonction utilisée par la lib CanDccBoosterRailCom.
 *
 *  Elle lit directement ADC1_CHANNEL_0 (GPIO36).
 *
 *  → C’est TON entrée RailCom locale.
 *  → Elle n’apparaît pas dans EXCC_Pins.h car les ADC internes
 *    ne passent pas par ce fichier.
 */
int16_t EXCC_BoosterHardware::readRailcomSample()
{
    return adc1_get_raw(ADC1_CHANNEL_0);
}
