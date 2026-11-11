/*
 * Module : main.cpp (EXCC)
 * Rôle   : Point d'entrée matériel du firmware EXCC (ESP32).
 */

#include <Arduino.h>
#include "EXCC_Main.h"
#include "EXCC_Config.h"
#include "esp_task_wdt.h"

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

static TaskHandle_t exsaTaskHandle = nullptr;

static constexpr int EXCC_WDT_TIMEOUT_SEC = 2;

void exsaTask(void *parameter)
{
#if EXCC_DEBUG
    Serial.println("[RTOS] Tâche EXCC démarrée");
#endif

    esp_task_wdt_add(nullptr);

    for (;;)
    {
        esp_task_wdt_reset();
        EXCC_Main::loop();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void setup()
{
#if EXCC_DEBUG
    Serial.begin(EXCC_UART_BAUDRATE);
    delay(50);
    Serial.println("\n[BOOT] EXCC démarrage (RTOS + WDT)...");
#endif

    EXCC_Main::begin();

    esp_task_wdt_init(EXCC_WDT_TIMEOUT_SEC, true);

    xTaskCreatePinnedToCore(
        exsaTask,
        "EXCC_Task",
        4096,
        nullptr,
        1,
        &exsaTaskHandle,
        APP_CPU_NUM);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}