#include "EXCC_Canton_WS2812.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

/*
 * Module : EXCC_Canton
 * Rôle   : Gestion des LEDs d’état du canton via PCA9685.
 *
 * Fonctionnement général :
 *   - Affiche l’état du canton :
 *       • OCCUPÉ      (rouge)
 *       • LIBRE       (vert)
 *       • MOUVEMENT   (orange, pulse court)
 *       • ERREUR      (rouge fixe ou clignotant)
 *
 *   - Gère deux mécanismes internes :
 *       • Animation de démarrage (séquence 4 LEDs)
 *       • Pulse mouvement (durée EXCC_CANTON_MOUVEMENT_MS)
 *
 *   - Le PCA9685 est partagé avec :
 *       • EXCC_Servo
 *       • EXCC_LedDirection
 *
 * Contraintes :
 *   - Aucune allocation dynamique
 *   - Fonctions très courtes (appelées dans la boucle 10–20 ms)
 *   - Animation non bloquante
 *
 * Notes :
 *   - L’animation masque temporairement l’état réel du canton.
 *   - Dès qu’elle est terminée, l’état LIBRE est affiché par défaut.
 */

void EXCC_Canton_WS2812::begin() noexcept
{
    animStep = 0;
    animTimer = millis();
    animEnCours = true;

    if (m_led)
        *m_led = CRGB::Black;
}

void EXCC_Canton_WS2812::update() noexcept
{
    const unsigned long now = millis();

    // ---------------------------------------------------------
    // Animation de démarrage (4 étapes)
    // ---------------------------------------------------------
    if (animEnCours)
    {
        if (now - animTimer > EXCC_CANTON_ANIM_STEP_MS)
        {
            animTimer = now;

            switch (animStep)
            {
            case 0: *m_led = CRGB::Red;    break; // OCCUPÉ
            case 1: *m_led = CRGB::Green;  break; // LIBRE
            case 2: *m_led = CRGB::Orange; break; // MOUVEMENT
            case 3: *m_led = CRGB::Red;    break; // ERREUR
            case 4:
                *m_led = CRGB::Green;      // État initial = LIBRE
                animEnCours = false;
                break;
            }

            if (animStep < 4)
                animStep++;
        }
        return;
    }

    // ---------------------------------------------------------
    // Pulse mouvement
    // ---------------------------------------------------------
    if (mouvementActif && now - mouvementTimer > EXCC_CANTON_MOUVEMENT_MS)
    {
        mouvementActif = false;
    }

    // ---------------------------------------------------------
    // Erreur clignotante
    // ---------------------------------------------------------
    if (erreurActive)
    {
        if ((now / 300) % 2)
            *m_led = CRGB::Red;
        else
            *m_led = CRGB::Black;
    }
}

void EXCC_Canton_WS2812::setOccupation(bool occupe) noexcept
{
    if (animEnCours || erreurActive || mouvementActif)
        return;

    *m_led = occupe ? CRGB::Red : CRGB::Green;
}

void EXCC_Canton_WS2812::pulseMouvement() noexcept
{
    if (animEnCours)
        return;

    mouvementActif = true;
    mouvementTimer = millis();
    *m_led = CRGB::Orange;
}

void EXCC_Canton_WS2812::setErreur(bool erreur) noexcept
{
    if (animEnCours)
        return;

    erreurActive = erreur;

    if (erreur)
        *m_led = CRGB::Red;
    else
        *m_led = CRGB::Green;
}