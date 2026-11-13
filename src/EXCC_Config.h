#pragma once
#include "EXCC_Pins.h"

/*
 * ============================================================
 *  EXCC_Config.h — Configuration globale EXCC (WS2812)
 * ------------------------------------------------------------
 *  Ce fichier regroupe toutes les constantes de configuration
 *  du firmware EXCC. Il définit :
 *
 *    - options générales
 *    - temporisations canton
 *    - paramètres WS2812 (signaux SNCF)
 *    - intensités LED directionnelles (PCA9685)
 *    - seuils de supervision de l’état du Booster distant
 *
 *  ⚠️ Aucun aspect SNCF ici.
 *     Aucun protocole ici.
 *     Aucun bitfield ici.
 *
 *  Les aspects SNCF sont définis dans :
 *      → Protocol.h (enum ExccAspect)
 * ============================================================
 */

/* ============================================================
 *  Options générales EXCC
 * ============================================================ */
#define EXCC_DEBUG true      // Debug série — TODO: repasser à false après validation
#define EXCC_UART_BAUDRATE 115200 // Vitesse du port série debug (idem monitor_speed)
#define EXCC_DEBOUNCE_MS 5   // Anti-rebond capteur présence
#define EXCC_MAX_ESSIEUX 200 // Limite sécurité comptage

/* ============================================================
 *  Temporisations EXCC_Canton
 * ------------------------------------------------------------
 *  - EXCC_CANTON_ANIM_STEP_MS : vitesse animation LED
 *  - EXCC_CANTON_MOUVEMENT_MS : durée LED "mouvement"
 * ============================================================ */
#define EXCC_CANTON_ANIM_STEP_MS 120
#define EXCC_CANTON_MOUVEMENT_MS 200

/* ============================================================
 *  Paramètres WS2812 — Signaux SNCF
 * ------------------------------------------------------------
 *  - EXCC_WS2812_GLOBAL_BRIGHTNESS :
 *        luminosité globale (0–255) appliquée par Ws2812Universal
 *
 *  - EXCC_WS2812_BLINK_PERIOD_MS :
 *        période de clignotement (ralent 60 / rappel 60 / défaut)
 * ============================================================ */
#define EXCC_WS2812_GLOBAL_BRIGHTNESS 128
#define EXCC_WS2812_BLINK_PERIOD_MS 500

/* ============================================================
 *  Intensité des LED directionnelles (0–255)
 * ------------------------------------------------------------
 *  Utilisées par EXCC_LedDirection (PCA9685).
 * ============================================================ */
#define EXCC_DIR_LED0_INTENSITE 255
#define EXCC_DIR_LED1_INTENSITE 255
#define EXCC_DIR_LED2_INTENSITE 255
#define EXCC_DIR_LED3_INTENSITE 255

/* ============================================================
 *  Seuils de supervision du Booster distant
 * ============================================================ */
#define EXCC_BOOSTER_MAX_COURANT_mA 1400
#define EXCC_BOOSTER_MIN_TENSION_mV 12000

/* ============================================================
 *  État de supervision envoyé au CantonController.
 * ============================================================ */
enum ExccBoosterEtat : uint8_t
{
    BOOSTER_OFF = 0,
    BOOSTER_OK = 1,
    BOOSTER_COURT_CIRCUIT = 2,
    BOOSTER_SOUS_TENSION = 3,
    BOOSTER_SURCHAUFFE = 4,
    BOOSTER_ERREUR = 5
};

/* ============================================================
 *  Détection d’occupation par courant (Booster Chemin de Fer du Chablais (CFC))
 * ------------------------------------------------------------
 *  L’occupation du canton est déterminée par la consommation
 *  de courant mesurée par le Booster et transmise par UART.
 *
 *  Hystérésis :
 *    - Si LIBRE  → devient OCCUPÉ si courant > EXCC_SEUIL_OCCUPE_mA
 *    - Si OCCUPÉ → reste OCCUPÉ tant que courant > EXCC_SEUIL_LIBRE_mA
 *
 *  Ces valeurs dépendent du matériel (loco, éclairage, bruit ADC).
 * ============================================================ */
#define EXCC_SEUIL_OCCUPE_mA 15 // au-dessus → OCCUPÉ
#define EXCC_SEUIL_LIBRE_mA 8   // en-dessous → LIBRE

/* ============================================================
 *  Aiguillages
 * ------------------------------------------------------------
 *  AIG_COUNT représente le nombre total d’aiguillages gérés.
 *
 *  Relation structurelle Chemin de Fer du Chablais (CFC) :
 *      1 aiguillage = 1 servo = 2 micro‑switchs
 *
 *  Cette valeur définit donc :
 *      - le nombre de servos dans EXCC_Servo
 *      - le nombre d’entrées surveillées dans EXCC_Switches
 *      - le nombre d’index envoyés au CC (F0/F1/F2 et trame 0x06)
 *
 *  Toute modification du nombre d’aiguillages doit se faire ici.
 * ============================================================ */
constexpr uint8_t AIG_COUNT = 6;

/*
 * ============================================================
 *  Aucun aspect ici !
 * ------------------------------------------------------------
 *  Les aspects SNCF sont définis dans :
 *      → Protocol.h (enum ExccAspect)
 * ============================================================
 */
