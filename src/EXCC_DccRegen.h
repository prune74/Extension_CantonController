#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include <stddef.h>
#include "driver/rmt.h"
#include "EXCC_Pins.h"

/* ============================================================
 * EXCC_DccRegen — régénération du créneau DCC par RMT TX canal 7
 *
 * Driver RMT legacy, clk_div = 2 → tick 25 ns (comme Ws2812Universal).
 * Émission non bloquante dans le ring buffer du driver : le CPU ne
 * cadence RIEN. NMRA S-9.1 (demi-bits étirés amont normalisés) :
 *   bit '1' : 2 × 58 µs  → 2320 ticks
 *   bit '0' : 2 × 100 µs → 4000 ticks
 *
 * ⚠️ EXCC : canal 7 avec mem_block_num = 1 OBLIGATOIRE (dernier
 * bloc mémoire de l'ESP32 ; Ws2812Universal occupe les canaux 0..6).
 * ============================================================ */
class EXCC_DccRegen
{
public:
    static constexpr uint16_t TICKS_HALF_ONE = 2320; // 58 µs @ 25 ns
    static constexpr uint16_t TICKS_HALF_ZER = 4000; // 100 µs @ 25 ns

    bool begin();
    bool busy();
    // bits[i] = 0 ou 1 → items RMT biphasés ; retourne le nb de bits acceptés
    size_t emit(const uint8_t *bits, size_t n);

private:
    static constexpr size_t MAX_EMIT = 32; // 32 bits = 32 items par appel
};