#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <Arduino.h>
#include "driver/gpio.h"

/*
 * ============================================================
 *  EXCC_Pins.h — Brochage firmware EXCC (CFC)
 * ============================================================
 */

/* ============================================================
   Quadrature Essieux
   ------------------------------------------------------------
   La quadrature H utilise GPIO12/14.
   ⚠️ GPIO12 = strap MTDI : pull-down 10 kΩ obligatoire au routage.
   ============================================================ */
static const gpio_num_t PIN_QUAD_H_A = GPIO_NUM_12; // ⚠️ strap MTDI, pull-down 10 k
static const gpio_num_t PIN_QUAD_H_B = GPIO_NUM_14;
static const gpio_num_t PIN_QUAD_AH_A = GPIO_NUM_34; // entrée pure
static const gpio_num_t PIN_QUAD_AH_B = GPIO_NUM_35; // entrée pure

/* ============================================================
   WS2812 — Signaux SNCF (H / AH)
   ------------------------------------------------------------
   7 strips au total → RMT canaux 0..6 (Ws2812Universal).
   ============================================================ */
static const gpio_num_t PIN_WS2812_H = GPIO_NUM_18;      // Feux H
static const gpio_num_t PIN_WS2812_H_OEIL = GPIO_NUM_19; // Œilleton H
static const gpio_num_t PIN_WS2812_AH = GPIO_NUM_23;      // Feux AH
static const gpio_num_t PIN_WS2812_AH_OEIL = GPIO_NUM_25; // Œilleton AH

/* ============================================================
   WS2812 — Directions
   ============================================================ */
static const gpio_num_t PIN_WS2812_DIR_H = GPIO_NUM_26;
static const gpio_num_t PIN_WS2812_DIR_AH = GPIO_NUM_27;

/* ============================================================
   WS2812 — Status (6 LEDs : canton, booster, railcom, télémétrie, CAN, switches)
   ============================================================ */
static const gpio_num_t PIN_WS2812_STATUS = GPIO_NUM_15;

/* ============================================================
   UART — lien EXCC vers le Booster externe
   ============================================================ */
static const gpio_num_t PIN_BOOSTER_UART_TX = GPIO_NUM_16;
static const gpio_num_t PIN_BOOSTER_UART_RX = GPIO_NUM_17;

/* ============================================================
   PCA9685 — Servos (I2C 0x40)
   ============================================================ */
static const uint8_t PCA_SERVO_H_1 = 0;
static const uint8_t PCA_SERVO_H_2 = 1;
static const uint8_t PCA_SERVO_H_3 = 2;
static const uint8_t PCA_SERVO_AH_1 = 3;
static const uint8_t PCA_SERVO_AH_2 = 4;
static const uint8_t PCA_SERVO_AH_3 = 5;

/* ============================================================
   MCP23017 — Micro-switchs aiguilles (I2C 0x20)
   ============================================================ */
static const uint8_t MCP23017_ADDR = 0x20;

static const uint8_t MCP_SW_H_1_DROIT = 0;
static const uint8_t MCP_SW_H_1_DEVIE = 1;
static const uint8_t MCP_SW_H_2_DROIT = 2;
static const uint8_t MCP_SW_H_2_DEVIE = 3;
static const uint8_t MCP_SW_H_3_DROIT = 4;
static const uint8_t MCP_SW_H_3_DEVIE = 5;

static const uint8_t MCP_SW_AH_1_DROIT = 6;
static const uint8_t MCP_SW_AH_1_DEVIE = 7;
static const uint8_t MCP_SW_AH_2_DROIT = 8;
static const uint8_t MCP_SW_AH_2_DEVIE = 9;
static const uint8_t MCP_SW_AH_3_DROIT = 10;
static const uint8_t MCP_SW_AH_3_DEVIE = 11;

static const gpio_num_t PIN_MCP23017_INT = GPIO_NUM_NC; // INT non utilisé

/* ============================================================
   I2C — bus partagé (PCA9685 0x40 + MCP23017 0x20)
   ============================================================ */
static const gpio_num_t PIN_I2C_SDA = GPIO_NUM_21;
static const gpio_num_t PIN_I2C_SCL = GPIO_NUM_22;

/* ============================================================
   CAN Command — TWAI natif vers le CantonController
   ============================================================ */
#define PIN_CAN_TX GPIO_NUM_4
#define PIN_CAN_RX GPIO_NUM_5
#define CAN_BITRATE 250000UL
