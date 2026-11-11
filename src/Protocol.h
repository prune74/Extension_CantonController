#pragma once
#include <stdint.h>

/*
 * ============================================================================
 *  Protocol.h — Gestion Canton
 * ============================================================================
 *  Structure modernisée :
 *    → une enum class par liaison CAN
 *    → aucune collision d’opcodes
 *    → lisibilité maximale
 *    → documentation naturelle du protocole
 * ============================================================================
 */

/* ============================================================================
 *  ENUM ASPECTS SNCF (EXCC → CC)
 * ==========================================================================*/
enum class ExccAspect : uint8_t
{
    ASPECT_CARRE = 0,
    ASPECT_CARRE_VIOLET,
    ASPECT_SEMAPHORE,
    ASPECT_AVERTISSEMENT,
    ASPECT_RALENTISSEMENT_30,
    ASPECT_RALENTISSEMENT_60,
    ASPECT_RAPPEL_30,
    ASPECT_RAPPEL_60,
    ASPECT_VOIE_LIBRE,
    ASPECT_MANOEUVRE,
    ASPECT_MASQUE,
};

/* ============================================================
 *  🟦 ENUM TYPE DE MATS
 * ============================================================
 */
enum SignalType : uint8_t
{
    SIG_ABSENT = 0, // Aucun signal présent physiquement

    TYPE_A = 1, // BAL (cible A), 3 feux
    TYPE_C = 2, // Carré + BAL (cible C), 5 feux
    TYPE_E = 3, // Ralentissement (cible E), 7 feux
    TYPE_G = 4, // Rappel (cible G), 9 feux
    TYPE_M = 5  // Manœuvre (cible M), 2 feux
};

/* ============================================================================
 *  COMMANDES EXCC → CC (29 bits)
 * ==========================================================================*/
enum class Cmd_EXCC_to_CC : uint16_t
{
    PONG = 0xD1,
    BOOSTER_INFO = 0xD2,
    POSITION_AIGUILLE = 0xD6,
    OCCUPATION = 0xD7,
    PONCTUEL_H = 0xD8,
    PONCTUEL_AH = 0xD9,
    RAILCOM_ADRESSE = 0xDA,
    CALIB_BOOSTER_INFO = 0xDB,
    ESSIEUX = 0xDC,
};

/* ============================================================================
 *  COMMANDES CC → EXCC (29 bits)
 * ==========================================================================*/
enum class Cmd_CC_to_EXCC : uint16_t
{
    SERVO_MOVE = 0xF0,
    SERVO_CONFIG = 0xF1,
    SERVO_TEST = 0xF2,
    RECALIBRER_BOOSTER = 0xF3,
    SET_SEUILS = 0xF4,
    BOOSTER_POWER = 0xF5,
    CONFIG_SIGNAUX = 0xF6,
    ASPECT_HORAIRE = 0xF7,
    ASPECT_ANTIHORAIRE = 0xF8,
    DIRECTION_HORAIRE = 0xF9,
    DIRECTION_ANTIHORAIRE = 0xFA,
    OCCUPATION_VOISINS = 0xFB,
    PING = 0xFC,
    CANTON_ID = 0xFD,
    PROFILE_VOIE = 0xFE,
};

/* ============================================================================
 *  COMMANDES CC ↔ CC (Exploitation ferroviaire)
 * ==========================================================================*/
enum class Cmd_CC_to_CC : uint16_t
{
    UPDATE_VOISINS = 0xE0,
    RESERVATION_LOCO = 0xE3,
    RAILCOM_VOISIN = 0xE5,
    ASPECT_VOISIN = 0xE7,
    AIGUILLAGE = 0xE9,
    MESURE_PREPARE = 0xEB, // CC0 → CCx : préparer la mesure de vitesse
    RECHERCHE_WAGON = 0xEC, // CC0 → CCx : Recherche directionnelle des wagons fugueurs
};

/* ============================================================================
 *  COMMANDES CC ↔ CC (Exploration topologique)
 * ==========================================================================*/
enum class Cmd_Exploration_CC : uint16_t
{
    DEMANDE_ID = 0xC0,
    UPDATE_MASQUE_AIG = 0xC1,
    ID_VOISIN = 0xC2,
};

/* ============================================================================
 *  COMMANDES ERM → CC
 * ==========================================================================*/
enum class Cmd_ERM_to_CC : uint16_t
{
    TEST_BUS_REPLY = 0xB3,
    REQUEST_ID_REPLY = 0xB5,
    WIFI_ON_OFF = 0xBD,
    EXPLORATION_ON_OFF = 0xBE,
    SAVE_ALL = 0xBF,
    RESTART_ALL = 0xBC,
    SET_PROFILE = 0x20,
    OFFLINE = 0xC3,
};

/* ============================================================================
 *  COMMANDES CC → ERM
 * ==========================================================================*/
enum class Cmd_CC_to_ERM : uint16_t
{
    TEST_BUS = 0xB2,
    REQUEST_ID = 0xB4,
};

/* ============================================================================
 *  COMMANDES CC → CLF (Centrale de Logique Ferroviaire)
 * ==========================================================================*/
enum class Cmd_CC_to_CLF : uint16_t
{
    MESURE_VITESSE = 0xA0,
    ESSIEUX_TRAIN = 0xA1,
};

/* ============================================================================
 * COMMANDES CLF → CC (Centrale de Logique Ferroviaire)
 ============================================================================*/
enum class Cmd_CLF_to_CC : uint16_t
{
    TRAIN_VALIDE        = 0x90, // train validé → silence epoch
    TRAIN_REARMER       = 0x91, // fin du silence
    DEMANDE_MESURE      = 0x92, // demande vitesse + essieux
    CLF_TIME            = 0x93, // synchronisation de l'heure (epoch)
    TRAIN_COMPOSITION   = 0x94, // composition complète du train
};

/* ============================================================================
 *  CODES INTERNES EXCC (non-CAN)
 * ==========================================================================*/
enum class ExccCode : uint8_t
{
    POS_DROIT = 0x00,
    POS_DEVIE = 0x01,
    POS_INDET = 0x02,
    POS_INCOHERENT = 0x03,

    PONCT_H_ACTIVE = 0x10,
    PONCT_H_INACTIVE = 0x11,
    PONCT_AH_ACTIVE = 0x12,
    PONCT_AH_INACTIVE = 0x13,

    OCC_ACTIVE = 0x30,
    OCC_LIBRE = 0x31,

    ETAT_OK = 0x00,
    ETAT_BLOQUE = 0x01,
    ETAT_ERREUR = 0x02,
};

/* ============================================================================
 *  COMMANDES GLOBALES 11 bits
 * ==========================================================================*/
enum class Cmd_Global11 : uint16_t
{
    HEARTBEAT = 0x200,
    EMERGENCY_STOP = 0x201,
    CLEAR_STOP = 0x202,
};

/* ============================================================================
 *  COMMANDES CAN LaBox (traction DCC NMRA)
 * ==========================================================================*/
enum class Cmd_CAN_LaBox : uint16_t
{
    SYSTEM_CONTROL = 0x00,    // Power ON/OFF, emergency stop global
    LOCO_SPEED = 0x04,        // Vitesse loco
    LOCO_DIRECTION = 0x05,    // Sens de marche
    LOCO_FUNCTION = 0x06,     // Fonctions F0..F28
    LOCO_WRITECV_MAIN = 0x08, // Écriture CV sur voie principale
};

/* ============================================================================
 *  ID Carte CLF - Centrale de Logique Ferrovière
 * ==========================================================================*/
constexpr uint16_t CLF_ID = 253;

/* ============================================================================
 *  ID Carte ERM - Exploration Reseau Maitre
 * ==========================================================================*/
constexpr uint16_t ERM_ID = 254;