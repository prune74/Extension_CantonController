/*
 * ============================================================================
 *  Module : EXCC_System (WS2812 Version)
 * ============================================================================
 */

#include "EXCC_System.h"

#include "EXCC_Pins.h"
#include "EXCC_Config.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

// Instances globales déclarées dans EXCC_Main.cpp
extern Adafruit_PWMServoDriver g_pca9685;

/* ============================================================================
 * init()
 * ----------------------------------------------------------------------------
 * PHASE 0 — Correction du crash au boot :
 *   Le PCA9685 n'était JAMAIS initialisé (aucun Wire.begin(), aucun
 *   g_pca9685.begin()) alors que EXCC_Servo::begin() appelait setPWM()
 *   immédiatement → transaction I2C sur un bus non ouvert →
 *   Guru Meditation LoadProhibited (EXCVADDR 0x0000000C).
 *
 * init() est désormais réduit à l'init système bas niveau (I2C + PCA9685).
 * Tout le reste de l'ordonnancement est géré par EXCC_Main::begin().
 * ============================================================================
 */
void EXCC_System::init() noexcept
{
    // Bus I2C (SDA 21 / SCL 22) — partagé PCA9685 (0x40) et futur digipot
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    delay(2); // tBUSFREE / stabilisation

    // PCA9685 — OBLIGATOIRE avant EXCC_Servo::begin()
    g_pca9685.begin();
}