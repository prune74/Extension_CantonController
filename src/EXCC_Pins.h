#pragma once
#include <Arduino.h>

/*
 * ============================================================
 *  EXCC_Pins.h — Mappage matériel officiel Discovery 2026
 *  Version optimisée ESP32‑DEVKIT V1 (redistribution complète)
 * ============================================================
 */

/* ============================================================
   UART EXCC ↔ CC (RS485) — Serial1
   ============================================================ */
static const int EXCC_UART_BAUDRATE = 115200;
static const gpio_num_t PIN_RS485_RX    = GPIO_NUM_16;
static const gpio_num_t PIN_RS485_TX    = GPIO_NUM_17;
static const gpio_num_t PIN_RS485_DE_RE = GPIO_NUM_12;   // OK en sortie

/* ============================================================
   Quadrature Essieux (ADC1 uniquement)
   ============================================================ */
static const gpio_num_t PIN_QUAD_H_A  = GPIO_NUM_32;  // ADC1_CH4
static const gpio_num_t PIN_QUAD_H_B  = GPIO_NUM_33;  // ADC1_CH5
static const gpio_num_t PIN_QUAD_AH_A = GPIO_NUM_34;  // entrée pure
static const gpio_num_t PIN_QUAD_AH_B = GPIO_NUM_35;  // entrée pure

/* ============================================================
   WS2812 — Signaux SNCF (H / AH)
   ============================================================ */
static const gpio_num_t PIN_WS2812_H       = GPIO_NUM_18; // Feux H
static const gpio_num_t PIN_WS2812_H_OEIL  = GPIO_NUM_19; // Œilleton H

static const gpio_num_t PIN_WS2812_AH      = GPIO_NUM_23; // Feux AH
static const gpio_num_t PIN_WS2812_AH_OEIL = GPIO_NUM_25; // Œilleton AH

/* ============================================================
   WS2812 — Directions
   ============================================================ */
static const gpio_num_t PIN_WS2812_DIR_H  = GPIO_NUM_26;
static const gpio_num_t PIN_WS2812_DIR_AH = GPIO_NUM_27;

/* ============================================================
   WS2812 — Status (6 LEDs)
   ============================================================ */
static const gpio_num_t PIN_WS2812_STATUS = GPIO_NUM_15;

/* ============================================================
   PCA9685 — Servos
   ============================================================ */
static const uint8_t PCA_SERVO_H_1  = 0;
static const uint8_t PCA_SERVO_H_2  = 1;
static const uint8_t PCA_SERVO_H_3  = 2;
static const uint8_t PCA_SERVO_AH_1 = 3;
static const uint8_t PCA_SERVO_AH_2 = 4;
static const uint8_t PCA_SERVO_AH_3 = 5;

/* ============================================================
   MCP23017 — Micro-switchs aiguilles
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

static const gpio_num_t PIN_MCP23017_INT = GPIO_NUM_17; // non utilisé

/* ============================================================
   Booster DRV8874 + PWM DCC + ADC1
   ============================================================ */
static const gpio_num_t PIN_DRV_NSLEEP = GPIO_NUM_13;
static const gpio_num_t PIN_DRV_PHASE  = GPIO_NUM_14;
static const gpio_num_t PIN_DRV_FAULT  = GPIO_NUM_2;

static const gpio_num_t PIN_DCC_PWM    = GPIO_NUM_23; // RMT/PWM OK

// ============================================================================
//  NOTE IMPORTANTE : ADC du Booster EXCC
//  -------------------------------------
//  Les trois mesures analogiques du Booster (courant, tension, RailCom HF)
//  sont lues EXCLUSIVEMENT dans EXCC_BoosterHardware via l’ADC1 de l’ESP32.
//
//      - GPIO32 → ADC1_CHANNEL_4 → IPROPI (courant)
//      - GPIO33 → ADC1_CHANNEL_5 → Tension voie
//      - GPIO36 → ADC1_CHANNEL_0 → RailCom HF (pendant le cutout)
//
//  → EXCC_CanBooster ne doit PAS définir ces pins.
//  → EXCC_CanBooster ne lit AUCUN ADC.
//  → Toute la gestion analogique est centralisée dans EXCC_BoosterHardware.
// ============================================================================

/* ============================================================
   CAN natif ESP32
   ============================================================ */
static const gpio_num_t PIN_CAN_RX = GPIO_NUM_4;
static const gpio_num_t PIN_CAN_TX = GPIO_NUM_5;
