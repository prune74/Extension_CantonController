#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <Arduino.h>
#include "driver/gpio.h"

/*
 * ============================================================
 *  EXCC_Pins.h — Mappage matériel officiel Chemin de Fer du Chablais (CFC)
 *  Version cible avec booster DRV8874 intégré (mode PWM IN1/IN2).
 *  PHASE 1 — brochage figé pour le routage de la carte.
 * ============================================================
 */

/* ============================================================
   Quadrature Essieux
   ------------------------------------------------------------
   PHASE 1 : la quadrature H passe de 32/33 à 12/14.
   GPIO32/33 sont désormais RÉSERVÉS à l'ADC booster (IPROPI/VRAIL).
   ⚠️ GPIO12 = strap MTDI : pull-down 10 kΩ obligatoire au routage.
   ============================================================ */
static const gpio_num_t PIN_QUAD_H_A = GPIO_NUM_12; // ⚠️ strap MTDI, pull-down 10 k
static const gpio_num_t PIN_QUAD_H_B = GPIO_NUM_14;
static const gpio_num_t PIN_QUAD_AH_A = GPIO_NUM_34; // entrée pure
static const gpio_num_t PIN_QUAD_AH_B = GPIO_NUM_35; // entrée pure

/* ============================================================
   WS2812 — Signaux SNCF (H / AH)
   ------------------------------------------------------------
   7 strips au total → RMT canaux 0..6 (FastLED).
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
   DRV8874 — mode PWM (PMODE strapé à 3,3 V)
   ------------------------------------------------------------
   IN1 = DCC régénéré (RMT TX canal 7)
   IN2 = même signal INVERSÉ via la matrice GPIO (gpio_matrix)
   Cutout    : IN1 = IN2 = 1 → BRAKE (rails au potentiel commun)
   Failsafe  : IN1 = IN2 = 0 → COAST (sorties Hi-Z)
   PHASE 1 : remplace PIN_DCC_PWM (16, conservé) et PIN_DRV_PHASE
   (14, libéré pour la quadrature H) de l'ancien mode PH/EN.
   ============================================================ */
static const gpio_num_t PIN_DRV_IN1 = GPIO_NUM_16;    // DCC RMT canal 7 → IN1
static const gpio_num_t PIN_DRV_IN2 = GPIO_NUM_17;    // même signal inversé → IN2
static const gpio_num_t PIN_DRV_NSLEEP = GPIO_NUM_13; // Veille driver (1 = actif)
static const gpio_num_t PIN_DRV_FAULT = GPIO_NUM_2;  // Défaut driver (actif bas)

/* ============================================================
   ADC booster (RÉSERVÉS — ne rien câbler d'autre dessus)
   ------------------------------------------------------------
   GPIO32 → ADC1_CHANNEL_4 → IPROPI (courant) via R 3,6 kΩ → GND
   GPIO33 → ADC1_CHANNEL_5 → Tension voie (pont diviseur 68 kΩ / 10 kΩ)
   ============================================================ */
static const gpio_num_t PIN_IPROPI = GPIO_NUM_32; // ADC1_CH4
static const gpio_num_t PIN_VRAIL = GPIO_NUM_33;  // ADC1_CH5

/* ============================================================
   RailCom — front-end carte (pont BAT54 + ampli TLV9151 + opto TLP2362)
   Sortie RAILCOM_ISO : signal LOGIQUE 3,3 V isolé galvaniquement.
   Décodage par UART2 RX à 250 kbit/s (input-only OK via matrice GPIO).
   ============================================================ */
static const gpio_num_t PIN_RAILCOM_ISO = GPIO_NUM_36; // UART2 RX

/* ============================================================
   RMT — allocation des canaux (ESP32 : 8 canaux / 8 blocs mémoire)
   ------------------------------------------------------------
   Canaux 0..6 : FastLED (7 strips WS2812)
   Canal 7     : DCC TX — ⚠️ mem_block_num = 1 OBLIGATOIRE
                (dernier bloc mémoire de l'ESP32 ; mem_block_num = 2
                échouerait, contrairement au dépôt DRV8874 canal 5).
   Aucun strip WS2812 supplémentable sans libérer un canal.
   ============================================================ */
static constexpr int RMT_DCC_CHANNEL = 7; // EXCC_DccRegen (Phase 2)

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
   I2C — bus partagé (PCA9685 0x40 + MCP23017 0x20 + digipot MCP4551 0x2E)
   ============================================================ */
static const gpio_num_t PIN_I2C_SDA = GPIO_NUM_21;
static const gpio_num_t PIN_I2C_SCL = GPIO_NUM_22;

/* Digipot MCP4551/4561 — profil tension voie HO 15 V / N 12 V (Phase 2) */
static const uint8_t DIGIPOT_ADDR = 0x2E;
static const uint8_t DIGIPOT_REG = 0x00;

/* ============================================================
   CAN — Bus booster (convertisseur DCC_CAN_RMT-Booster + CantonController)
   TWAI natif, 500 kbit/s, 11 bits — identique au convertisseur.
   ============================================================ */
#define PIN_CAN_TX GPIO_NUM_4
#define PIN_CAN_RX GPIO_NUM_5
#define CAN_BITRATE 500000UL

/* ============================================================
   ABANDONNÉ en Phase 1 (supprimé volontairement)
   ------------------------------------------------------------
   - PIN_DCC_PWM (GPIO16) → devient PIN_DRV_IN1
   - PIN_DRV_PHASE (GPIO14) → libéré pour la quadrature H (mode IN1/IN2)
   - MCP2515 / bus EXCC 29 bits (CS 13, INT 14, SPI 18/19/23) :
     conflits avec WS2812 (SPI) et DRV8874 (CS/INT). Le bus service/
     commande 29 bits vivra sur une carte dédiée (reporté).
     → EXCC_CanConfig doit rester à 1 seul bus (TWAI), le code
       qui tente d'initialiser CAN1/MCP2515 doit être retiré en Phase 2.
   ============================================================ */