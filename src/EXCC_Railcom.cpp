#include "EXCC_RailCom.h"
#include "driver/uart.h"

bool EXCC_RailCom::begin()
{
    const uart_config_t cfg = {
        .baud_rate = 250000,             // NMRA S-9.3.2 : 250 kbit/s
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,       // parité PAIRE — à valider à l'oscillo
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    if (uart_param_config(UART_NUM_2, &cfg) != ESP_OK)
        return false;
    // RX sur RAILCOM_ISO (GPIO36, input-only OK via matrice d'entrées)
    if (uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, PIN_RAILCOM_ISO,
                     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK)
        return false;
    return uart_driver_install(UART_NUM_2, 256, 0, 0, nullptr, 0) == ESP_OK;
}

void EXCC_RailCom::arm()
{
    uart_flush_input(UART_NUM_2); // purge : on ne garde que la fenêtre à venir
}

uint16_t EXCC_RailCom::collect()
{
    uint8_t bytes[MAX_BYTES];
    uint8_t n = 0;

    // Les octets sont arrivés PENDANT le cutout (~488 µs) : lecture non
    // bloquante de ce qui est déjà dans le buffer du driver.
    size_t avail = 0;
    uart_get_buffered_data_len(UART_NUM_2, &avail);
    if (avail > MAX_BYTES)
        avail = MAX_BYTES;
    if (avail > 0)
        n = uart_read_bytes(UART_NUM_2, bytes, avail, 0);

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