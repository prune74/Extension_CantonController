/*
 * ============================================================================
 *  Module : EXCC_Runtime (WS2812 Version)
 *  Rôle   : Boucle d’exécution principale EXCC (hors Booster).
 *
 *  Fonctionnement général (Gestion Canton 2026) :
 *    - Appelé en continu par EXCC_Main::loop()
 *    - Exécute toutes les tâches temps réel EXCC :
 *         1) Mise à jour signaux SNCF WS2812 (H et AH)
 *         2) Mise à jour canton WS2812 (occupation + mouvement + erreur)
 *         3) Mise à jour aiguilles (anti‑blocage)
 *
 *  Notes :
 *    - Le Booster tourne dans sa propre tâche FreeRTOS.
 *    - Les feux directionnels WS2812 n’ont pas de logique interne :
 *         → pas de clignotement
 *         → pas d’animation
 *         → pas de mise à jour dans la boucle runtime
 * ============================================================================
 */

#include "EXCC_Runtime.h"

#include "EXCC_Signaux_WS2812.h"

#include "EXCC_Ponctuel.h"
#include "EXCC_Occupation.h"
#include "EXCC_Compteur.h"
#include "EXCC_Booster_WS2812.h"

#include "EXCC_Canton_WS2812.h"
#include "EXCC_Switches.h"

#include <FastLED.h>
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Protection plateforme : ce module nécessite l'ESP32
// ---------------------------------------------------------------------------
#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

// Instances globales déclarées dans EXCC_Main.cpp
extern EXCC_Signaux_WS2812 signauxH;
extern EXCC_Signaux_WS2812 signauxAH;

extern EXCC_Canton_WS2812 cantonWS;

extern EXCC_Booster_WS2812 booster;
/* ============================================================================
 * update()
 * ----------------------------------------------------------------------------
 * Boucle principale EXCC (hors booster).
 * Appelée en continu par EXCC_Main::loop().
 * ============================================================================
 */
void EXCC_Runtime::update() noexcept
{
   uint32_t now = millis();

   /* --------------------------------------------------------
      Mise à jour Occupation (fusion des sources)
      -------------------------------------------------------- */

   // Courant du booster
   EXCC_Occupation::majCourant(booster.readCurrent_mA());

   // Compteur d’essieux
   EXCC_Occupation::majCompteur(EXCC_Compteur::compteurGlobal());

   // Ponctuels H / AH
   EXCC_Occupation::majPonctuels(
       EXCC_Ponctuel::estActifH(),
       EXCC_Ponctuel::estActifAH());

   // Calcul final de l’occupation
   EXCC_Occupation::calculerOccupationFinale();

   /* --------------------------------------------------------
      Mise à jour des signaux SNCF WS2812
      -------------------------------------------------------- */
   signauxH.update(now);
   signauxAH.update(now);

   /* --------------------------------------------------------
      Mise à jour canton WS2812
      -------------------------------------------------------- */
   cantonWS.update();

   /* --------------------------------------------------------
      Mise à jour aiguilles (anti‑blocage)
      -------------------------------------------------------- */
   EXCC_Switches::update();

   /* --------------------------------------------------------
      Application WS2812
      -------------------------------------------------------- */
   FastLED.show();
}
