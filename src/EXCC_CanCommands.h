#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>

class EXCC_CanCommands
{
public:
    static void begin();
    static void process();
    static uint32_t lastRxMs();
    static bool hasReceivedFrame();
};
