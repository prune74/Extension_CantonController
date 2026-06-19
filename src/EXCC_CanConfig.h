#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include "CanConfig.h"
#include "EXCC_Pins.h"

struct EXCC_CanConfig : public CanConfigProvider
{

    uint8_t busCount() const override
    {
        return 1;
    }

    const CanBusConfig &bus(uint8_t index) const override
    {
        static CanBusConfig cfg;

        cfg.enabled = true;
        cfg.speed = 500000; // 500 kbps

        // TWAI interne ESP32
        cfg.tx_pin = PIN_CAN_TX;
        cfg.rx_pin = PIN_CAN_RX;

        // MCP2515 désactivé
        cfg.cs_pin = GPIO_NUM_NC;
        cfg.int_pin = GPIO_NUM_NC;
        cfg.sck_pin = GPIO_NUM_NC;
        cfg.mosi_pin = GPIO_NUM_NC;
        cfg.miso_pin = GPIO_NUM_NC;

        cfg.quartz = 0;
        cfg.tolerance = 0;
        cfg.loopback = false;

        return cfg;
    }
};
