#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>

class EXCC_CanCommands
{
public:
    // Appelé régulièrement dans loop()
    static void process();
};
