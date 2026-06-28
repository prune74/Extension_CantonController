#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <stdint.h>

class EXCC_CAN_CC
{
public:
    static void sendPong(uint16_t idCanton);
    static void envoyerPonctuelH(uint16_t idCanton, bool actif);
    static void envoyerPonctuelAH(uint16_t idCanton, bool actif);
    static void envoyerOccupation(uint16_t idCanton, bool occ);
    static void envoyerBooster(uint16_t idCanton, uint8_t etat, uint8_t courant, uint8_t tension);
    static void envoyerRailcom(uint16_t idCanton, uint8_t type, uint16_t adresse);
    static void envoyerCalibBooster(uint16_t idCanton, uint16_t seuilLibre_mA, uint16_t seuilOccupe_mA);
    static void envoyerPositionAiguille(uint16_t idCanton, uint8_t servoIndex, uint8_t position, uint8_t etat);
};
