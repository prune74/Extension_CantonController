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
 *    - seuils de sécurité du Booster Gestion Canton 2026
 *    - paramètres analogiques DRV8874 (IPROPI)
 *
 *  ⚠️ Aucun aspect SNCF ici.
 *     Aucun protocole ici.
 *     Aucun bitfield ici.
 *
 *  Les aspects SNCF sont définis dans :
 *      → Exploration_Protocol.h (enum ExsaAspect)
 * ============================================================
 */

/* ============================================================
 *  Options générales EXCC
 * ============================================================ */
#define EXCC_DEBUG false     // Debug série
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
 *        luminosité globale (0–255) appliquée par FastLED
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
 *  Sécurité Booster Gestion Canton 2026
 * ------------------------------------------------------------
 *  Seuils de protection voie :
 *
 *  - EXCC_BOOSTER_MAX_COURANT_mA
 *      Courant max avant coupure (court-circuit local)
 *
 *  - EXCC_BOOSTER_MIN_TENSION_mV
 *      Tension min avant coupure (voie OFF / alim faible)
 *
 *  - EXCC_BOOSTER_PHASE_TOLERANCE
 *      Tolérance inversion de phase DCC (0 = strict)
 *
 *  - EXCC_BOOSTER_ENABLE_GLOBAL_PROTECTION
 *      true  = coupe si un autre booster signale un défaut
 *      false = ignore les défauts des autres boosters
 *
 *  - EXCC_BOOSTER_ENABLE_GLOBAL_CUTOUT
 *      true  = coupe si cutout global actif
 *      false = ignore le cutout global
 * ============================================================ */
#define EXCC_BOOSTER_MAX_COURANT_mA 1400
#define EXCC_BOOSTER_MIN_TENSION_mV 12000
#define EXCC_BOOSTER_PHASE_TOLERANCE 0
#define EXCC_BOOSTER_ENABLE_GLOBAL_PROTECTION true
#define EXCC_BOOSTER_ENABLE_GLOBAL_CUTOUT true

/* ============================================================
 *  Mesure courant via DRV8874 (IPROPI)
 * ------------------------------------------------------------
 *  Formule :
 *      I = V_IPROPI / (R_IPROPI * A_IPROPI)
 *
 *  Avec :
 *      R_IPROPI = 3,6 kΩ
 *      A_IPROPI = 455 µA/A
 *
 *  Permet une mesure précise du courant voie.
 * ============================================================ */
#define EXCC_IPROPI_R_OHMS 3600.0f
#define EXCC_IPROPI_GAIN_A_PER_A 0.000455f

/* ============================================================
 *  Mesure tension voie DCC (pont diviseur)
 * ------------------------------------------------------------
 *  Pont diviseur :
 *      R1 = 68 kΩ (haut)
 *      R2 = 10 kΩ (bas)
 *
 *  Formule :
 *      Vrail = Vmes * (R1 + R2) / R2
 * ============================================================ */
#define EXCC_ADC_VOLTAGE_R1_OHMS 68000.0f
#define EXCC_ADC_VOLTAGE_R2_OHMS 10000.0f

// Tension de référence ADC ESP32
#define EXCC_ADC_VREF 3.3f

// Facteur de conversion (pré-calculé)
#define EXCC_ADC_VOLTAGE_FACTOR \
    ((EXCC_ADC_VOLTAGE_R1_OHMS + EXCC_ADC_VOLTAGE_R2_OHMS) / EXCC_ADC_VOLTAGE_R2_OHMS)

/* ============================================================
 *  État global du Booster Gestion Canton 2026
 * ------------------------------------------------------------
 *  Utilisé dans EXCC_BoosterCore pour :
 *    - télémétrie
 *    - sécurité
 *    - trames 0x07 vers CC
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
 *  Détection d’occupation par courant (Booster Gestion Canton 2026)
 * ------------------------------------------------------------
 *  L’occupation du canton est déterminée par la consommation
 *  de courant mesurée via IPROPI (DRV8874).
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
 *  Relation structurelle Gestion Canton 2026 :
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
 *      → Exploration_Protocol.h (enum ExsaAspect)
 * ============================================================
 */
