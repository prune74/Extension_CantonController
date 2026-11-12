#include "EXCC_DccRegen.h"

bool EXCC_DccRegen::begin()
{
    rmt_config_t cfg = RMT_DEFAULT_CONFIG_TX((gpio_num_t)PIN_DRV_IN1,
                                             (rmt_channel_t)RMT_DCC_CHANNEL);
    cfg.clk_div = 2;   // tick 25 ns
    cfg.mem_block_num = 1; // EXCC : canal 7 = DERNIER bloc mémoire (≠ dépôt DRV8874 canal 5/2 blocs)
    if (rmt_config(&cfg) != ESP_OK)
        return false;
    // Ring buffer 4096 octets = 1024 items ≈ 1024 demi-paires
    if (rmt_driver_install(cfg.channel, 4096, 0) != ESP_OK)
        return false;
    return true;
}

bool EXCC_DccRegen::busy()
{
    return rmt_wait_tx_done((rmt_channel_t)RMT_DCC_CHANNEL, 0) == ESP_ERR_TIMEOUT;
}

size_t EXCC_DccRegen::emit(const uint8_t *bits, size_t n)
{
    rmt_item32_t items[MAX_EMIT];
    size_t k = 0;
    if (n > MAX_EMIT)
        n = MAX_EMIT;

    for (size_t i = 0; i < n; i++)
    {
        const uint16_t t = bits[i] ? TICKS_HALF_ONE : TICKS_HALF_ZER;
        items[k].level0 = 0;
        items[k].duration0 = t; // demi-bit bas
        items[k].level1 = 1;
        items[k].duration1 = t; // demi-bit haut → biphasé exact
        k++;
    }

    return (rmt_write_items((rmt_channel_t)RMT_DCC_CHANNEL, items, k, false) == ESP_OK)
               ? n : 0;
}