#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>

class EXCC_Tension_Booster
{
public:
    static void begin() noexcept;
    static void setTrackProfile(bool ho) noexcept;

private:
    static void writeDigipot(uint8_t value) noexcept;
};
