// EXCC_BoosterCore.cpp
#include "EXCC_BoosterCore.h"
#include "EXCC_CanBooster.h"
#include "EXCC_Booster_WS2812.h"
#include "EXCC_Calibration.h"
#include "EXCC_CAN_CC.h"
#include "EXCC_Main.h"
#include "EXCC_Callbacks.h"   // pour g_idCC

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

static TaskHandle_t s_boosterTaskHandle = nullptr;
static bool s_boosterEnabled = true;
extern uint8_t g_idCC;

/*
 * ============================================================================
 *  PHASE 2 — le timer HF RailCom (10 µs) a été SUPPRIMÉ :
 *  l'échantillonnage ADC a été remplacé par le décodage UART2 250 kbit/s
 *  (EXCC_RailCom, hooks dans EXCC_Booster_WS2812). Zéro CPU pour la
 *  capture RailCom, plus aucune interruption à 100 kHz.
 * ============================================================================
 */

/*
 * ============================================================================
 *  EXCC_BoosterCore::startTask()
 * ============================================================================
 */
void EXCC_BoosterCore::startTask()
{
    if (s_boosterTaskHandle != nullptr)
        return;

    xTaskCreatePinnedToCore(
        EXCC_BoosterCore::taskEntry,
        "BoosterCore",
        4096,
        nullptr,
        3,
        &s_boosterTaskHandle,
        APP_CPU_NUM);
}

/*
 * ============================================================================
 *  EXCC_BoosterCore::setEnabled()
 * ============================================================================
 */
void EXCC_BoosterCore::setEnabled(bool enabled) noexcept
{
    s_boosterEnabled = enabled;
    booster.setEnabled(enabled);
}

/*
 * ============================================================================
 *  EXCC_BoosterCore::taskEntry()
 * ============================================================================
 */
void EXCC_BoosterCore::taskEntry(void *param)
{
    (void)param;

    // PHASE 0 — booster.begin() SUPPRIMÉ ici :
    // le booster est déjà initialisé dans EXCC_Main::begin(), AVANT le
    // lancement de cette tâche.

    EXCC_CanBooster::begin();

    EXCC_Calibration::init();
    EXCC_Calibration::start();

    esp_task_wdt_add(nullptr);

    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1);

    uint32_t lastBoosterSend = 0;
    bool calibEnvoyee = false;

    for (;;)
    {
        esp_task_wdt_reset();

        EXCC_CanBooster::process();

        if (s_boosterEnabled)
            booster.update();

        EXCC_Calibration::process();

        if (EXCC_Calibration::isDone() && !calibEnvoyee)
        {
            calibEnvoyee = true;
            EXCC_CAN_CC::envoyerCalibBooster(
                g_idCC,
                EXCC_Calibration::getSeuilLibre(),
                EXCC_Calibration::getSeuilOccupe());
        }

        if (EXCC_Calibration::isRunning())
            calibEnvoyee = false;

        uint32_t now = millis();
        if (now - lastBoosterSend >= 200)
        {
            lastBoosterSend = now;

            uint16_t courant = booster.readCurrent_mA();
            uint16_t tension = booster.readVoltage_mV();

            ExccBoosterEtat etat;

            if (!s_boosterEnabled)
                etat = BOOSTER_OFF;
            else if (tension < EXCC_BOOSTER_MIN_TENSION_mV)
                etat = BOOSTER_SOUS_TENSION;
            else if (courant > EXCC_BOOSTER_MAX_COURANT_mA)
                etat = BOOSTER_COURT_CIRCUIT;
            else if (booster.isThermalFault())
                etat = BOOSTER_SURCHAUFFE;
            else
                etat = BOOSTER_OK;

            EXCC_CAN_CC::envoyerBooster(
                g_idCC,
                etat,
                min<uint16_t>(courant / 10, 255),
                min<uint16_t>(tension / 100, 255));
        }

        vTaskDelayUntil(&lastWakeTime, period);
    }
}