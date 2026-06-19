#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>

class EXCC_BoosterCore
{
public:
    static void startTask();
    static void setEnabled(bool enabled) noexcept;

    // Point d’entrée de la tâche FreeRTOS
    static void taskEntry(void *param);
};
