#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

/*
 * ============================================================
 *  EXCC_Quadrature — Capture brute A/B (Exploration 2026)
 * ------------------------------------------------------------
 *  Rôle :
 *    - Lire les signaux quadrature A/B en temps réel
 *    - ISR ultra courte (aucune logique métier)
 *    - Bufferisation via queue FreeRTOS
 *    - Logique de décodage faite dans EXCC_Compteur
 *
 *  Architecture :
 *    - Une instance par capteur (H et AH)
 *    - ISR dédiée par instance
 *    - Queue interne (32 événements)
 *
 *  Important :
 *    - Ce module ne calcule PAS le delta
 *    - Ce module ne gère PAS le compteur
 *    - Ce module ne gère PAS le RS485
 * ============================================================
 */

class EXCC_Quadrature
{
public:
    EXCC_Quadrature();

    // Initialisation : pins A/B
    void begin(gpio_num_t pinA, gpio_num_t pinB);

    // Lecture non bloquante d’un événement A/B
    bool lireEvenement(uint8_t &etat);

    // Installation des interruptions
    void installerInterruptions();

    // Création de la queue interne
    void initQueue();

    // ISR interne (appelée par isrQuadH / isrQuadAH)
    void IRAM_ATTR isr();

private:
    gpio_num_t m_pinA;
    gpio_num_t m_pinB;

    QueueHandle_t m_queue;
};

// Instances globales (déclarées dans EXCC_Quadrature.cpp)
extern EXCC_Quadrature quadH;
extern EXCC_Quadrature quadAH;
