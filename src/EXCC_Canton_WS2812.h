#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <FastLED.h>
#include <stdint.h>
#include "EXCC_Config.h"

/*
 * EXCC_Canton_WS2812
 * -------------------
 * Version WS2812 du module Canton.
 * API identique à EXCC_Canton (PCA9685).
 */

class EXCC_Canton_WS2812
{
public:
    EXCC_Canton_WS2812(CRGB *led) : m_led(led) {}

    void begin() noexcept;
    void update() noexcept;

    void setOccupation(bool occupe) noexcept;
    void pulseMouvement() noexcept;
    void setErreur(bool erreur) noexcept;
    void setVoisins(uint8_t voisins) noexcept;

private:
    CRGB *m_led;

    // Animation de démarrage
    uint8_t animStep = 0;
    unsigned long animTimer = 0;
    bool animEnCours = true;

    // Pulse mouvement
    bool mouvementActif = false;
    unsigned long mouvementTimer = 0;

    // Erreur
    bool erreurActive = false;
};
