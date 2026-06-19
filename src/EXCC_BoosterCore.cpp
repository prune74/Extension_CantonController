// EXCC_BoosterCore.cpp
#include "EXCC_BoosterCore.h"
#include "EXCC_CanBooster.h"
#include "EXCC_Booster_WS2812.h"
#include "EXCC_Calibration.h"
#include "EXCC_UartTx.h"
#include "EXCC_Config.h"
#include "EXCC_Main.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>

/*
 * ============================================================================
 *  SECTION : Gestion de la tâche FreeRTOS du Booster
 * ============================================================================
 */
static TaskHandle_t s_boosterTaskHandle = nullptr;

// Activation/désactivation globale du booster
static bool s_boosterEnabled = true;

/*
 * ============================================================================
 *  SECTION : Timer HF RailCom
 * ============================================================================
 */
static hw_timer_t *s_railcomTimer = nullptr;

void IRAM_ATTR railcomTimerISR()
{
    if (EXCC_CanBooster::isCutoutActive())
    {
        booster.feedRailcomSample();
    }
}

/*
 * ============================================================================
 *  EXCC_BoosterCore::startTask()
 * ============================================================================
 */
void EXCC_BoosterCore::startTask()
{
    if (s_boosterTaskHandle != nullptr)
        return;

    // --- Timer HF RailCom (100 kHz) ---
    s_railcomTimer = timerBegin(0, 80, true); // 80 MHz / 80 = 1 MHz
    timerAttachInterrupt(s_railcomTimer, &railcomTimerISR, true);
    timerAlarmWrite(s_railcomTimer, 10, true); // 10 µs → 100 kHz
    timerAlarmEnable(s_railcomTimer);

    // --- Tâche Booster 1 ms ---
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

    // Initialisation des modules
    EXCC_CanBooster::begin();
    booster.begin();

    EXCC_Calibration::init();
    EXCC_Calibration::start();

    // Ajout au watchdog
    esp_task_wdt_add(nullptr);

    // Timing 1 ms
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1);

    uint32_t lastBoosterSend = 0;
    bool calibEnvoyee = false;

    for (;;)
    {
        esp_task_wdt_reset();

        // 1) Lecture des trames CAN Booster
        EXCC_CanBooster::process();

        // 2) Mise à jour du booster (DCC, cutout, RailCom, sécurité)
        if (s_boosterEnabled)
            booster.update();

        // 3) Calibration automatique
        EXCC_Calibration::process();

        // Envoi des seuils une fois la calibration terminée
        if (EXCC_Calibration::isDone() && !calibEnvoyee)
        {
            calibEnvoyee = true;
            EXCC_UartTx::envoyerCalibBooster(
                EXCC_Calibration::getSeuilLibre(),
                EXCC_Calibration::getSeuilOccupe());
        }

        if (EXCC_Calibration::isRunning())
            calibEnvoyee = false;

        // 4) Envoi de la télémétrie toutes les 200 ms
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

            EXCC_UartTx::envoyerBooster(
                etat,
                min<uint16_t>(courant / 10, 255),
                min<uint16_t>(tension / 100, 255));
        }

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
