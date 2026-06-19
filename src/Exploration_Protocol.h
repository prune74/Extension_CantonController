#pragma once
#include <stdint.h>

/*
 * ============================================================
 *  EXPLORATION 2026 — PROTOCOLE OFFICIEL
 * ------------------------------------------------------------
 *  Fichier unique partagé entre :
 *    • ExplorationMaster (Master)
 *    • Satellites Autonomes (CC)
 *    • Extensions Satellites (EXCC)
 *
 *  Contient :
 *    • Aspects SNCF (enum)
 *    • Protocole CC ↔ EXCC (PROTO_xx)
 *    • Protocole Master ↔ CC (CMD_xx)
 *
 *  Objectif :
 *    → Garantir une cohérence totale du protocole Exploration 2026
 *    → Éviter les divergences entre firmwares
 *    → Faciliter la maintenance et l’évolution
 * ============================================================
 */

/* ============================================================
 *  🟦 ENUM DES ASPECTS SNCF — 1 octet
 * ============================================================
 */

enum ExsaAspect : uint8_t
{
    ASPECT_CARRE = 0,         // 🔴 Arrêt absolu
    ASPECT_SEMAPHORE,         // 🔴 Arrêt
    ASPECT_AVERTISSEMENT,     // 🟡 Ralentir
    ASPECT_RALENTISSEMENT_30, // 🟡⚠️ 30 km/h
    ASPECT_RALENTISSEMENT_60, // 🟡⚠️ 60 km/h
    ASPECT_RAPPEL_30,         // 🟡🔁 Rappel 30
    ASPECT_RAPPEL_60,         // 🟡🔁 Rappel 60
    ASPECT_VOIE_LIBRE,        // 🟢 Voie libre
    ASPECT_MANOEUVRE,         // 🔵 Manoeuvre
    ASPECT_MASQUE,            // ⚫ Masqué
    ASPECT_DEFAUT             // ⚠️ Défaut
};

/* ============================================================
 *  🟧 PROTOCOLE CC ↔ EXCC — Format série 0xAA / OPCODE
 * ============================================================
 */

#define PROTO_SYNC_BYTE 0xAA // Octet de synchronisation

/* --- Supervision --- */
#define PROTO_PING 0x32 // CC → EXCC : ping
#define PROTO_PONG 0x33 // EXCC → CC : pong

/* --- Topologie / Configuration --- */
#define PROTO_E4_TOPOLOGIE_CAN 0xE4  // Topologie CAN
#define PROTO_E5_CONFIG_SIGNAUX 0xE5 // Configuration signaux

/* --- Aspects SNCF --- */
#define PROTO_E6_ASPECT_HORAIRE 0xE6     // Aspect côté H
#define PROTO_E7_ASPECT_ANTIHORAIRE 0xE7 // Aspect côté AH

/* --- Feux directionnels --- */
#define PROTO_E8_DIRECTION_HORAIRE 0xE8
#define PROTO_E9_DIRECTION_ANTIHORAIRE 0xE9

/* --- Occupation voisins --- */
#define PROTO_EA_OCCUPATION_VOISINS 0xEA

/* --- Servos --- */
#define PROTO_F0_SERVO_MOVE 0xF0
#define PROTO_F1_SERVO_CONFIG 0xF1
#define PROTO_F2_SERVO_TEST 0xF2

/* --- Booster (CC → EXCC) --- */
#define PROTO_F3_RECALIBRER_BOOSTER 0xF3 // Recalibration automatique
#define PROTO_F4_SET_SEUILS 0xF4         // Seuils calibrés (4 octets)
#define PROTO_F5_BOOSTER_POWER 0xF5      // Booster ON/OFF (0=OFF, 1=ON)

/* ============================================================
 *  🟩 EXCC → CC : Informations remontées
 * ============================================================
 */

#define PROTO_03_H_PONCTUEL 0x03        // Retour ponctuel H
#define PROTO_03_AH_PONCTUEL 0x04       // Retour ponctuel AH
#define PROTO_04_OCCUPATION 0x05        // Occupation canton
#define PROTO_05_COMPTEUR_ESSIEUX 0x06  // Compteur d'essieux
#define PROTO_06_POSITION_AIGUILLE 0x07 // Position réelle
#define PROTO_07_BOOSTER 0x08           // État booster
#define PROTO_08_RAILCOM_ADRESSE 0x09   // Adresse RailCom
#define PROTO_09_CALIB_BOOSTER 0x0A     // Seuils calibrés

/* ============================================================
 *  🟪 Codes associés EXCC → CC
 * ============================================================
 */

/* --- PONCTUEL --- */
#define PROTO_PONCT_H_ACTIVE 0x10
#define PROTO_PONCT_H_INACTIVE 0x11
#define PROTO_PONCT_AH_ACTIVE 0x12
#define PROTO_PONCT_AH_INACTIVE 0x13

/* --- Occupation --- */
#define PROTO_OCC_ACTIVE 0x30
#define PROTO_OCC_LIBRE 0x31

/* --- Position aiguille --- */
#define PROTO_POS_DROIT 0x00
#define PROTO_POS_DEVIE 0x01
#define PROTO_POS_INDET 0x02
#define PROTO_POS_INCOHERENT 0x03

/* --- États sécurité --- */
#define PROTO_ETAT_OK 0x00
#define PROTO_ETAT_BLOQUE 0x01
#define PROTO_ETAT_ERREUR 0x02

/* ============================================================
 *  🟦 PROTOCOLE MASTER ↔ CC — Bus CAN 29 bits
 * ============================================================
 *
 *  Ces commandes sont envoyées par :
 *    • ExplorationMaster (Master)
 *    • Satellites (CC / EXCC)
 *
 *  Elles utilisent le format CAN étendu Märklin 29 bits.
 * ============================================================
 */

/* --- Master → Satellites --- */
#define CMD_WIFI_ON_OFF 0xBD        // Active/désactive WiFi
#define CMD_EXPLORATION_ON_OFF 0xBE // Active/désactive Exploration
#define CMD_SAVE_ALL 0xBF           // Sauvegarde globale
#define CMD_RESTART_ALL 0xBC        // Redémarrage global
#define CMD_SET_PROFILE 0x20        // Profil voie (0=N, 1=HO)

/* --- Satellites → Master --- */
#define CMD_SAT_TEST_BUS 0xB2         // Test bus CAN
#define CMD_SAT_TEST_BUS_REPLY 0xB3   // Réponse test bus
#define CMD_SAT_REQUEST_ID 0xB4       // Demande ID
#define CMD_SAT_REQUEST_ID_REPLY 0xB5 // Réponse ID

/* ============================================================
 *  🟥 EXPLORATION CAN 11 bits — Messages globaux Master ↔ CC
 * ============================================================
 *
 *  Ces IDs ne suivent PAS le format 29 bits Märklin.
 *  Ils sont utilisés pour la supervision globale.
 */

/* Heartbeat CC → Master */
#define EXPLORATION_CAN_ID_HEARTBEAT 0x200 // DLC=2 : [ID_H][ID_L]

/* STOP global Master → CC (puis CC → EXCC via RS485) */
#define EXPLORATION_CAN_ID_EMERGENCY_STOP 0x201 // DLC=0 : STOP global

/* (optionnel) CLEAR STOP global Master → CC */
#define EXPLORATION_CAN_ID_CLEAR_STOP 0x202 // DLC=0 : levée STOP
