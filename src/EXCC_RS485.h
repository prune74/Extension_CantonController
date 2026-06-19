#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include <Arduino.h>   
#include <driver/gpio.h>

class EXCC_RS485
{
public:
    static void begin(HardwareSerial &serial,
                      uint32_t baudrate,
                      gpio_num_t pinDE_RE) noexcept;

    static void send(const uint8_t *data, uint8_t len) noexcept;

    static HardwareSerial &uart() noexcept;
};
