// EXCC_BoosterCore.h
#pragma once
#include <stdint.h>

class EXCC_BoosterCore
{
public:
    static void startTask();
    static void setEnabled(bool enabled) noexcept;

    // Point d’entrée de la tâche FreeRTOS
    static void taskEntry(void *param);
};
