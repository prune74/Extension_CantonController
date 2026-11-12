// EXCC_BoosterCore.cpp
#include "EXCC_BoosterCore.h"
#include "EXCC_CanBooster.h"
#include "EXCC_Booster_WS2812.h"
#include "EXCC_BoosterHardware.h"
#include "EXCC_Calibration.h"
#include "EXCC_CAN_CC.h"
#include "EXCC_Main.h"
#include "EXCC_Callbacks.h" // pour g_idCC
#include "CAN_DCC_RMT-Booster_Types.h"
#include "driver/twai.h"    // BENCH : état interne du contrôleur CAN

#include "CanMsg.h"
#include "CanBus.h"

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
 *  PHASE 2 — timer HF RailCom supprimé (UART2 prend le relais).
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

void EXCC_BoosterCore::setEnabled(bool enabled) noexcept
{
    s_boosterEnabled = enabled;
    booster.setEnabled(enabled);
}

void EXCC_BoosterCore::taskEntry(void *param)
{
    (void)param;

    EXCC_CanBooster::begin();

#if EXCC_DEBUG
    // BENCH : loopback logiciel CanUniversal — les trames émises
    // ressortent côté receive(). (Le loopback MATÉRIEL de TwaiBackend
    // est inopérant en v3.1.0 : drapeau self jamais posé → v3.1.1.)
    CanBus::bus(0).enableSoftwareLoopback();
#endif

    EXCC_Calibration::init();
    EXCC_Calibration::start();

    esp_task_wdt_add(nullptr);

    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(1);

    uint32_t lastBoosterSend = 0;
    bool calibEnvoyee = false;

    // --- BENCH (loopback) : cadence des blocs et des rapports ---
    uint32_t dernierBenchBloc = 0;
    uint32_t dernierBenchRapport = 0;
    uint32_t bitsEmisPrecedents = 0;
#if EXCC_DEBUG && EXCC_BENCH_RMT_INJECTION
    uint8_t benchDccBit = 0;
#endif

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

#if EXCC_DEBUG && EXCC_BENCH_RMT_INJECTION
        // ------------------------------------------------------------------
        // Test ESP32 seul : contourne le CAN et les sécurités du booster.
        // ------------------------------------------------------------------
        if (now - dernierBenchBloc >= 1)
        {
            dernierBenchBloc = now;
            booster.benchEmitDccBit(benchDccBit);
            benchDccBit ^= 1U;
        }

        if (now - dernierBenchRapport >= 2000)
        {
            dernierBenchRapport = now;

            const uint32_t bits = EXCC_BoosterHardware::bitsEmis();
            const uint32_t delta = bits - bitsEmisPrecedents;
            bitsEmisPrecedents = bits;

            // État brut du contrôleur TWAI (CanUniversal ne l'expose pas)
            twai_status_info_t st = {};
            twai_get_status_info(&st);
            const BoosterTelemetry &boosterTelemetry = booster.getTelemetry();

            Serial.printf("[BENCH] bus=%s rx=%lu bits=%lu (+%lu/2s) | booster state=%u error=%u V=%u mV I=%u mA | twai state=%u txerr=%lu rxerr=%lu\n",
                          CanBus::bus(0).isValid() ? "OK" : "INVALIDE",
                          (unsigned long)EXCC_CanBooster::rxCount(),
                          (unsigned long)bits,
                          (unsigned long)delta,
                          static_cast<unsigned>(boosterTelemetry.state),
                          static_cast<unsigned>(boosterTelemetry.error),
                          static_cast<unsigned>(boosterTelemetry.voltage_mV),
                          static_cast<unsigned>(boosterTelemetry.current_mA),
                          st.state, // 1=RUNNING, 2=BUS_OFF, 3=RECOVERING
                          (unsigned long)st.tx_error_counter,
                          (unsigned long)st.rx_error_counter);

            // BENCH : bus-off → guérison manuelle (CanUniversal ne récupère
            // pas en mode loopback). Le bus repart au prochain send().
            if (st.state == TWAI_STATE_BUS_OFF)
            {
                twai_stop();
                twai_initiate_recovery();
                Serial.println("[BENCH] BUS_OFF -> recuperation TWAI lancee");
            }
        }
#endif

        vTaskDelayUntil(&lastWakeTime, period);
    }
}