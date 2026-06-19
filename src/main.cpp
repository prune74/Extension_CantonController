/*
 * Module : main.cpp (EXCC)
 * Rôle   : Point d’entrée matériel du firmware EXCC (ESP32).
 *
 * Fonctionnement général :
 *   - Configure le port série debug (optionnel)
 *   - Initialise EXCC via EXCC_Main::begin()
 *   - Configure le watchdog matériel (WDT)
 *   - Crée la tâche FreeRTOS dédiée à EXCC
 *
 * Architecture :
 *   - Une seule tâche EXCC (exsaTask)
 *   - EXCC_Main::loop() est exécuté ~1000 fois par seconde
 *   - Le Booster, s’il est activé, tourne dans CC propre tâche
 *
 * Contraintes temps réel :
 *   - La tâche EXCC doit impérativement reset le watchdog
 *   - Aucun blocage dans EXCC_Main::loop()
 *   - La tâche tourne sur un cœur dédié (APP_CPU_NUM)
 *
 * Notes :
 *   - La loop() Arduino est volontairement inutilisée.
 *   - Toute la logique EXCC tourne dans FreeRTOS.
 */

#include <Arduino.h>
#include "EXCC_Main.h"
#include "EXCC_Config.h"
#include "esp_task_wdt.h"

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

static TaskHandle_t exsaTaskHandle = nullptr;

// Timeout watchdog (en secondes)
static constexpr int EXCC_WDT_TIMEOUT_SEC = 2;

/* ============================================================
 * exsaTask()
 * ------------------------------------------------------------
 * Tâche FreeRTOS principale EXCC.
 *
 * Étapes :
 *   1) Enregistre la tâche auprès du watchdog
 *   2) Boucle infinie :
 *        - reset watchdog
 *        - exécute EXCC_Main::loop()
 *        - délai 1 ms (cadence ~1 kHz)
 *
 * Cette tâche est le cœur du runtime EXCC.
 * ============================================================ */
void exsaTask(void *parameter)
{
#if EXCC_DEBUG
    Serial.println("[RTOS] Tâche EXCC démarrée");
#endif

    // Enregistrement auprès du watchdog
    esp_task_wdt_add(nullptr);

    for (;;)
    {
        esp_task_wdt_reset();         // sécurité temps réel
        EXCC_Main::loop();            // logique EXCC
        vTaskDelay(pdMS_TO_TICKS(1)); // cadence 1 kHz
    }
}

/* ============================================================
 * setup()
 * ------------------------------------------------------------
 * Point d’entrée Arduino.
 *
 * Étapes :
 *   1) Port série debug (optionnel)
 *   2) Initialisation EXCC (EXCC_Main::begin)
 *   3) Initialisation watchdog
 *   4) Création de la tâche EXCC
 *
 * Le Booster sera lancé automatiquement par EXCC_Main::begin()
 * si le DIP Booster est activé.
 * ============================================================ */
void setup()
{
#if EXCC_DEBUG
    Serial.begin(EXCC_UART_BAUDRATE);
    delay(50);
    Serial.println("\n[BOOT] EXCC démarrage (RTOS + WDT)...");
#endif

    // Initialisation complète EXCC (système + modules)
    EXCC_Main::begin();

    // Watchdog matériel
    esp_task_wdt_init(EXCC_WDT_TIMEOUT_SEC, true);

    // Création de la tâche EXCC
    xTaskCreatePinnedToCore(
        exsaTask,        // fonction
        "EXCC_Task",     // nom
        4096,            // stack
        nullptr,         // paramètre
        1,               // priorité
        &exsaTaskHandle, // handle
        APP_CPU_NUM      // cœur CPU
    );
}

/* ============================================================
 * loop()
 * ------------------------------------------------------------
 * Boucle Arduino standard : inutilisée.
 *
 * EXCC tourne exclusivement dans FreeRTOS.
 * ============================================================ */
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
