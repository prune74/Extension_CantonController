/*
 * Module : EXCC_Servo
 * Rôle   : Pilotage avancé des servos d’aiguillage Gestion Canton 2026.
 *
 * Fonctionnement général :
 *   - 6 servos maximum (PCA9685)
 *   - Position droite / déviée en microsecondes (converties en PWM)
 *   - Interpolation progressive (vitesse configurable)
 *   - Modes avancés :
 *        • Servo OFF : PWM = 0 quand position atteinte
 *        • Soft‑Start : accélération progressive du mouvement
 *        • Deadband : zone morte anti‑oscillation
 *
 * Flux entrants :
 *   - move()      : commande CC F0
 *   - configure() : commande CC F1
 *   - test()      : commande CC F2
 *
 * Flux sortants :
 *   - PWM vers PCA9685
 *   - Accesseurs pour EXCC_Switches (sécurité anti‑blocage)
 *
 * Contraintes temps réel :
 *   - Aucun malloc
 *   - update() doit être très court (appelé ~1 kHz)
 *   - Interpolation déterministe
 *
 * Notes :
 *   - Le module ne gère pas la sécurité : EXCC_Switches surveille
 *     les mouvements anormaux ou bloqués.
 */

#include "EXCC_Servo.h"
#include "EXCC_Pins.h"
#include "EXCC_Config.h"

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

/*
 * ============================================================
 *  EXCC_Servo.cpp — Version améliorée 2026
 * ------------------------------------------------------------
 *  Ajouts :
 *    - Mode Servo OFF (PWM = 0 quand position atteinte)
 *    - Soft‑Start (accélération progressive)
 *    - Deadband (zone morte anti‑oscillation)
 * ============================================================
 */

extern Adafruit_PWMServoDriver g_pca9685;

namespace
{
    // ============================================================
    //  Servos:
    // ============================================================

    // Canaux PCA9685 associés aux servos
    constexpr uint8_t SERVO_CHANNEL[AIG_COUNT] = {
        PCA_SERVO_H_1,  // Servo 1 Horaire
        PCA_SERVO_H_2,  // Servo 2 Horaire
        PCA_SERVO_H_3,  // Servo 3 Horaire
        PCA_SERVO_AH_1, // Servo 1 Anti‑Horaire
        PCA_SERVO_AH_2, // Servo 2 Anti‑Horaire
        PCA_SERVO_AH_3  // Servo 3 Anti‑Horaire
    };

    // Plage microsecondes → PWM
    constexpr uint16_t SERVO_US_MIN = 800;
    constexpr uint16_t SERVO_US_MAX = 2400;
    constexpr uint16_t SERVO_US_RANGE = SERVO_US_MAX - SERVO_US_MIN;

    // Modes avancés
    bool servoOffEnabled = true;
    bool softStartEnabled = true;
    uint16_t deadband = 3;

    // Conversion µs → PWM PCA9685 (0–4095)
    inline uint16_t usToPwm(uint16_t us)
    {
        if (us < SERVO_US_MIN)
            us = SERVO_US_MIN;
        else if (us > SERVO_US_MAX)
            us = SERVO_US_MAX;

        return (uint32_t)(us - SERVO_US_MIN) * 4095u / SERVO_US_RANGE;
    }

    // Structure interne d’un servo
    struct ServoData
    {
        uint16_t posDroit_pwm;
        uint16_t posDevie_pwm;
        uint16_t speed_pwm;
        uint16_t current_pwm;
        uint16_t target_pwm;
        uint16_t softStartCounter = 1;
    };

    ServoData servos[AIG_COUNT];

    inline bool indexValide(uint8_t index)
    {
        return index < AIG_COUNT;
    }

    inline void appliquerPWM(uint8_t index)
    {
        g_pca9685.setPWM(SERVO_CHANNEL[index], 0, servos[index].current_pwm);
    }
}

/* ============================================================
 * begin()
 * ------------------------------------------------------------
 * Initialise les servos :
 *   - positions par défaut = 1500 µs
 *   - vitesse par défaut
 *   - PWM appliqué immédiatement
 * ============================================================ */
void EXCC_Servo::begin() noexcept
{
    for (uint8_t i = 0; i < AIG_COUNT; ++i)
    {
        servos[i].posDroit_pwm = usToPwm(1500);
        servos[i].posDevie_pwm = usToPwm(1500);
        servos[i].speed_pwm = 10;

        servos[i].current_pwm = servos[i].posDroit_pwm;
        servos[i].target_pwm = servos[i].posDroit_pwm;

        servos[i].softStartCounter = 1;

        appliquerPWM(i);
    }
}

/* ============================================================
 * update() — interpolation 1 kHz
 * ============================================================ */
void EXCC_Servo::update() noexcept
{
    for (uint8_t i = 0; i < AIG_COUNT; ++i)
    {
        uint16_t cur = servos[i].current_pwm;
        uint16_t tgt = servos[i].target_pwm;

        // Zone morte : position atteinte
        if (abs((int)cur - (int)tgt) <= deadband)
        {
            servos[i].current_pwm = tgt;

            if (servoOffEnabled)
                g_pca9685.setPWM(SERVO_CHANNEL[i], 0, 0);
            else
                appliquerPWM(i);

            continue;
        }

        // Vitesse de base
        uint16_t step = servos[i].speed_pwm;

        // Soft‑Start : accélération progressive
        if (softStartEnabled)
        {
            if (servos[i].softStartCounter < step)
                servos[i].softStartCounter++;

            step = servos[i].softStartCounter;
        }

        // Interpolation vers la cible
        if (cur < tgt)
        {
            uint16_t delta = tgt - cur;
            servos[i].current_pwm = (delta <= step) ? tgt : cur + step;
        }
        else
        {
            uint16_t delta = cur - tgt;
            servos[i].current_pwm = (delta <= step) ? tgt : cur - step;
        }

        appliquerPWM(i);
    }
}

/* ============================================================
 * F0 — move()
 * ============================================================ */
void EXCC_Servo::move(uint8_t index, uint8_t direction) noexcept
{
    if (!indexValide(index))
        return;

    servos[index].target_pwm =
        (direction == 0) ? servos[index].posDroit_pwm
                         : servos[index].posDevie_pwm;

    servos[index].softStartCounter = 1;
}

/* ============================================================
 * F1 — configure()
 * ============================================================ */
void EXCC_Servo::configure(uint8_t index,
                           uint16_t posDroit_us,
                           uint16_t posDevie_us,
                           uint16_t speed_us) noexcept
{
    if (!indexValide(index))
        return;

    servos[index].posDroit_pwm = usToPwm(posDroit_us);
    servos[index].posDevie_pwm = usToPwm(posDevie_us);

    // Conversion vitesse µs → PWM step
    servos[index].speed_pwm =
        (uint32_t)speed_us * 4095u / SERVO_US_RANGE / 50u;

    if (servos[index].speed_pwm < 1)
        servos[index].speed_pwm = 1;

    servos[index].current_pwm = servos[index].target_pwm;
    servos[index].softStartCounter = 1;
}

/* ============================================================
 * F2 — test()
 * ============================================================ */
void EXCC_Servo::test(uint8_t index) noexcept
{
    if (!indexValide(index))
        return;

    uint16_t mid = (servos[index].posDroit_pwm + servos[index].posDevie_pwm) / 2;

    servos[index].target_pwm =
        (servos[index].current_pwm < mid)
            ? servos[index].posDevie_pwm
            : servos[index].posDroit_pwm;

    servos[index].softStartCounter = 1;
}

/* ============================================================
 * Accesseurs
 * ============================================================ */
bool EXCC_Servo::isMoving(uint8_t index) noexcept
{
    return indexValide(index) && servos[index].current_pwm != servos[index].target_pwm;
}

uint16_t EXCC_Servo::getCurrentPwm(uint8_t index) noexcept
{
    return indexValide(index) ? servos[index].current_pwm : 0;
}

uint16_t EXCC_Servo::getTargetPwm(uint8_t index) noexcept
{
    return indexValide(index) ? servos[index].target_pwm : 0;
}

/* ============================================================
 * Modes avancés
 * ============================================================ */
void EXCC_Servo::enableServoOff(bool enable) noexcept
{
    servoOffEnabled = enable;
}

void EXCC_Servo::enableSoftStart(bool enable) noexcept
{
    softStartEnabled = enable;
}

void EXCC_Servo::setDeadband(uint16_t value) noexcept
{
    deadband = value;
}
