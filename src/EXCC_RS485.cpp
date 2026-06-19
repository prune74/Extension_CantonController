#include "EXCC_RS485.h"
#include "EXCC_Pins.h"

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

namespace
{
    // UART matériel utilisé pour le RS485
    HardwareSerial *g_uart = nullptr;

    // Broche DE/RE du transceiver RS485
    gpio_num_t g_pinDE_RE = GPIO_NUM_NC;
}

/* ============================================================
 * begin()
 * ============================================================ */
void EXCC_RS485::begin(HardwareSerial &serial,
                       uint32_t baudrate,
                       gpio_num_t pinDE_RE) noexcept
{
    g_uart = &serial;
    g_pinDE_RE = pinDE_RE;

    pinMode(g_pinDE_RE, OUTPUT);
    digitalWrite(g_pinDE_RE, LOW); // réception par défaut

    g_uart->begin(baudrate, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
}

/* ============================================================
 * uart()
 * ------------------------------------------------------------
 * Retourne une référence vers l’UART matériel.
 * Utilisé par EXCC_UartRx pour lire les trames CC.
 * ============================================================ */
HardwareSerial &EXCC_RS485::uart() noexcept
{
    return *g_uart;
}

/* ============================================================
 * send()
 * ============================================================ */
void EXCC_RS485::send(const uint8_t *data, uint8_t len) noexcept
{
    if (!g_uart)
        return;

    // Passage en émission
    digitalWrite(g_pinDE_RE, HIGH);
    delayMicroseconds(8);

    g_uart->write(data, len);
    g_uart->flush(); // attendre la fin réelle de l’envoi

    delayMicroseconds(8);

    // Retour en réception
    digitalWrite(g_pinDE_RE, LOW);
}
