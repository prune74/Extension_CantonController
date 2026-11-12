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
        return 1; // PHASE 1 : un seul bus TWAI (booster + CantonController).
                  // Le bus 29 bits MCP2515 est reporté sur une carte dédiée.
    }

    const CanBusConfig &bus(uint8_t index) const override
    {
        static CanBusConfig cfg;

        switch (index)
        {
        case 0: // Bus 0 : TWAI interne — convertisseur DCC ↔ EXCC ↔ CantonController
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

            cfg.loopback  = true;
            return cfg;
        }

        // Bus invalide
        static CanBusConfig invalid;
        return invalid;
    }
};