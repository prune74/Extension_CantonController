#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include "EXCC_Config.h"
#include "CanConfig.h"
#include "EXCC_Pins.h"

class EXCC_CanConfig : public CanConfigProvider
{
public:
    uint8_t busCount() const override
    {
        return 2; // 🟢 Deux bus CAN comme sur le CC
    }

    const CanBusConfig &bus(uint8_t index) const override
    {
        static CanBusConfig cfg;

        switch (index)
        {
        case 0: // 🟦 Bus 0 : TWAI interne (lien ERM ↔ Booster)
            cfg.enabled   = true;
            cfg.speed     = CAN_BITRATE;

            cfg.tx_pin    = PIN_CAN_TX;
            cfg.rx_pin    = PIN_CAN_RX;

            cfg.cs_pin    = GPIO_NUM_NC;
            cfg.int_pin   = GPIO_NUM_NC;
            cfg.sck_pin   = GPIO_NUM_NC;
            cfg.mosi_pin  = GPIO_NUM_NC;
            cfg.miso_pin  = GPIO_NUM_NC;

            cfg.quartz    = 0;
            cfg.tolerance = 0;
            cfg.loopback  = false;
            return cfg;

        case 1: // 🟧 Bus 1 : MCP2515 externe (lien CC ↔ EXCC)
            cfg.enabled   = true;
            cfg.speed     = CAN_BITRATE_MCP2515;

            cfg.tx_pin    = GPIO_NUM_NC;
            cfg.rx_pin    = GPIO_NUM_NC;

            cfg.cs_pin    = PIN_EXCC_CS;
            cfg.int_pin   = PIN_EXCC_INT;

            cfg.sck_pin   = PIN_EXCC_SCK;
            cfg.mosi_pin  = PIN_EXCC_MOSI;
            cfg.miso_pin  = PIN_EXCC_MISO;

            cfg.quartz    = QUARTZ_MCP2515;
            cfg.tolerance = 0;
            cfg.loopback  = false;
            return cfg;
        }

        // Bus invalide
        static CanBusConfig invalid;
        return invalid;
    }
};
