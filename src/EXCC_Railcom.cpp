#include "EXCC_RailCom.h"
#include "EXCC_Config.h"
#include <Arduino.h>
#include "esp_err.h"
#include "driver/uart.h"

bool EXCC_RailCom::begin()
{
    ready = false;

    const uart_config_t cfg = {
        .baud_rate = 250000,             // NMRA S-9.3.2 : 250 kbit/s
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,       // parité PAIRE — à valider à l'oscillo
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    esp_err_t err = uart_param_config(UART_NUM_2, &cfg);
    if (err != ESP_OK)
    {
#if EXCC_DEBUG
        Serial.printf("[RAILCOM] uart_param_config: %s\n", esp_err_to_name(err));
#endif
        return false;
    }

    // RX sur RAILCOM_ISO (GPIO36, input-only OK via matrice d'entrées)
    err = uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, PIN_RAILCOM_ISO,
                       UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK)
    {
#if EXCC_DEBUG
        Serial.printf("[RAILCOM] uart_set_pin: %s\n", esp_err_to_name(err));
#endif
        return false;
    }

    err = uart_driver_install(UART_NUM_2, 256, 0, 0, nullptr, 0);
    if (err != ESP_OK)
    {
#if EXCC_DEBUG
        Serial.printf("[RAILCOM] uart_driver_install: %s\n", esp_err_to_name(err));
#endif
        return false;
    }

    ready = true;
    return true;
}

void EXCC_RailCom::arm()
{
    if (!ready)
        return;

    const esp_err_t err = uart_flush_input(UART_NUM_2);
    if (err != ESP_OK)
    {
#if EXCC_DEBUG
        Serial.printf("[RAILCOM] uart_flush_input: %s\n", esp_err_to_name(err));
#endif
    }
}

uint16_t EXCC_RailCom::collect()
{
    if (!ready)
        return 0;

    uint8_t bytes[MAX_BYTES];

    // Les octets sont arrivés PENDANT le cutout (~488 µs) : lecture non
    // bloquante de ce qui est déjà dans le buffer du driver.
    size_t avail = 0;
    esp_err_t err = uart_get_buffered_data_len(UART_NUM_2, &avail);
    if (err != ESP_OK)
    {
#if EXCC_DEBUG
        Serial.printf("[RAILCOM] uart_get_buffered_data_len: %s\n", esp_err_to_name(err));
#endif
        return 0;
    }

    if (avail > MAX_BYTES)
        avail = MAX_BYTES;
    if (avail == 0)
        return 0;

    const int readCount = uart_read_bytes(UART_NUM_2, bytes, static_cast<uint32_t>(avail), 0);
    if (readCount < 0)
    {
#if EXCC_DEBUG
        Serial.printf("[RAILCOM] uart_read_bytes: erreur (%d)\n", readCount);
#endif
        return 0;
    }

    const uint8_t n = static_cast<uint8_t>(readCount);
    if (n == 0)
        return 0;
    frames++;

    // 1er paquet : CH1/CH2 = adresse de la loco
    uint16_t addr = decode(bytes[0], (n >= 2) ? bytes[1] : 0);
    if (addr != 0)
    {
        valid++;
        return addr;
    }

    // 2e paquet éventuel (certains décodeurs répètent l'adresse)
    if (n >= 4)
    {
        addr = decode(bytes[2], bytes[3]);
        if (addr != 0)
        {
            valid++;
            return addr;
        }
    }
    return 0;
}

// Reconstruction de l'adresse — principe Locoduino
uint16_t EXCC_RailCom::decode(uint8_t ch1, uint8_t ch2)
{
    if (ch1 == 0 || ch1 == 0xFF)
        return 0;                          // trame invalide
    if (ch2 == 0)
        return ch1;                        // adresse courte (1..127)
    const uint16_t addr = ((uint16_t)ch2 << 7) | ch1; // adresse longue
    if (addr < 128 || addr > 10239)
        return 0;                          // hors plage DCC
    return addr;
}