#pragma once
#include <Arduino.h>

/*
 * ============================================================
 *  EXCC_Pins.h — Mappage matériel officiel Gestion Canton 2026
 * ------------------------------------------------------------
 *  Ce fichier définit **toutes les broches physiques** utilisées
 *  par l’EXCC :
 *
 *    - UART RS485 (CC ↔ EXCC)
 *    - Quadrature essieux
 *    - WS2812 (mât SNCF)
 *    - PCA9685 (LED canton, direction, servos)
 *    - MCP23017 (micro‑switchs, DIP, présence)
 *    - Booster (DRV8801 + ADC + RailCom)
 *    - CAN natif ESP32
 *
 *  ⚠️ Ce fichier est la **source de vérité unique** du hardware.
 *     Toute modification doit être synchronisée avec le PCB.
 * ============================================================
 */

/* ============================================================
   UART EXCC ↔ CC (RS485)
   ------------------------------------------------------------
   - RX/TX : UART matériel (Serial1)
   - DE/RE : contrôle half‑duplex du transceiver RS485
   ============================================================ */
static const int EXCC_UART_BAUDRATE = 115200;
static const gpio_num_t PIN_RS485_RX = GPIO_NUM_25;
static const gpio_num_t PIN_RS485_TX = GPIO_NUM_26;
static const gpio_num_t PIN_RS485_DE_RE = GPIO_NUM_12;

// ============================================================
// Quadrature H (sens horaire)
// ============================================================
static const gpio_num_t PIN_QUAD_H_A = GPIO_NUM_14;
static const gpio_num_t PIN_QUAD_H_B = GPIO_NUM_27;

// ============================================================
// Quadrature AH (sens antihoraire)
// ============================================================
static const gpio_num_t PIN_QUAD_AH_A = GPIO_NUM_26;
static const gpio_num_t PIN_QUAD_AH_B = GPIO_NUM_25;

/* ============================================================
   WS2812 — Signaux SNCF (H / AH)
   ------------------------------------------------------------
   Chaque mât possède :
     - 1 strip "feux" (9 LED)
     - 1 strip "œilleton" (1 LED)
     - 1 strip "Direction" (4 LED)
   ============================================================ */
static const gpio_num_t PIN_WS2812_H = GPIO_NUM_18;      // Feux mât H
static const gpio_num_t PIN_WS2812_H_OEIL = GPIO_NUM_19; // Œilleton H

static const gpio_num_t PIN_WS2812_AH = GPIO_NUM_21;      // Feux mât AH
static const gpio_num_t PIN_WS2812_AH_OEIL = GPIO_NUM_22; // Œilleton AH

static const gpio_num_t PIN_WS2812_DIR_H = GPIO_NUM_21;  // Direction mât H
static const gpio_num_t PIN_WS2812_DIR_AH = GPIO_NUM_22; // Direction mât AH

static const gpio_num_t PIN_WS2812_CANTON = GPIO_NUM_22; // Direction mât AH
/* ============================================================
   PCA9685 — SORTIES (PWM)
   ------------------------------------------------------------
   Module 16 canaux utilisé pour :
     - Feux directionnels (4 LED)
     - LED canton (4 états)
     - Servos (3 aiguilles)
     - LED erreur Booster
   ============================================================ */

// LED erreur Booster (rouge)
static const uint8_t PCA_BOOSTER_ERROR = 7;

// LED canton (OCCUPÉ / LIBRE / MOUVEMENT / ERREUR)
static const uint8_t PCA_CANTON_OCCUPE = 8;
static const uint8_t PCA_CANTON_LIBRE = 9;
static const uint8_t PCA_CANTON_MOUVEMENT = 10;
static const uint8_t PCA_CANTON_ERREUR = 11;

// Servos (3 aiguilles)
static const uint8_t PCA_SERVO_H_1 = 12;
static const uint8_t PCA_SERVO_H_2 = 13;
static const uint8_t PCA_SERVO_H_3 = 14;
static const uint8_t PCA_SERVO_AH_1 = 15;
static const uint8_t PCA_SERVO_AH_2 = 16;
static const uint8_t PCA_SERVO_AH_3 = 17;

/* ============================================================
   MCP23017 — ENTRÉES UNIQUEMENT
   ------------------------------------------------------------
   Utilisé pour :
     - Micro‑switchs d’aiguilles
     - DIP H/AH + DIP Booster
     - Capteur présence
   ============================================================ */
static const uint8_t MCP23017_ADDR = 0x20;

// Micro-switchs aiguilles (DROIT / DEVIÉ)
static const uint8_t MCP_SW_H_1_DROIT = 0;   // Aiguille 1 Horaire
static const uint8_t MCP_SW_H_1_DEVIE = 1;   // Aiguille 1 Deviée
static const uint8_t MCP_SW_H_2_DROIT = 2;   // Aiguille 2 Horaire
static const uint8_t MCP_SW_H_2_DEVIE = 3;   // Aiguille 2 Deviée
static const uint8_t MCP_SW_H_3_DROIT = 4;   // Aiguille 3 Horaire
static const uint8_t MCP_SW_H_3_DEVIE = 5;   // Aiguille 3 Deviée
static const uint8_t MCP_SW_AH_1_DROIT = 6;  // Aiguille 1 Anti‑Horaire
static const uint8_t MCP_SW_AH_1_DEVIE = 7;  // Aiguille 1 Anti‑Horaire
static const uint8_t MCP_SW_AH_2_DROIT = 8;  // Aiguille 2 Anti‑Horaire
static const uint8_t MCP_SW_AH_2_DEVIE = 9;  // Aiguille 2 Anti‑Horaire
static const uint8_t MCP_SW_AH_3_DROIT = 10; // Aiguille 3 Anti‑Horaire
static const uint8_t MCP_SW_AH_3_DEVIE = 11; // Aiguille 3 Anti‑Horaire

// Interruption MCP23017 (non utilisée dans EXCC 2026)
static const gpio_num_t PIN_MCP23017_INT = GPIO_NUM_17;

/* ============================================================
   Booster Gestion Canton 2026 — ESP32 (temps réel)
   ------------------------------------------------------------
   Pilotage DRV8874 + mesures ADC + détection RailCom. :
     - nSLEEP : enable driver
     - PHASE  : sens DCC
     - FAULT  : retour erreur driver

   PWM DCC :
     - PIN_DCC_PWM : signal DCC haute fréquence
   ============================================================ */
static const gpio_num_t PIN_DRV_NSLEEP = GPIO_NUM_19;
static const gpio_num_t PIN_DRV_PHASE = GPIO_NUM_21;
static const gpio_num_t PIN_DRV_FAULT = GPIO_NUM_22;

static const gpio_num_t PIN_DCC_PWM = GPIO_NUM_23;

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
   CAN Booster (natif ESP32)
   ------------------------------------------------------------
   Utilisé pour la topologie Gestion Canton 2026.
   ============================================================ */
static const gpio_num_t PIN_CAN_RX = GPIO_NUM_4;
static const gpio_num_t PIN_CAN_TX = GPIO_NUM_5;
