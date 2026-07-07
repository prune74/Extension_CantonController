/*
 * Railcom.h — Gestion Canton 2026
 * ---------------------------------------------------------------------------
 * Réception de l’adresse RailCom détectée par l’Extension Canton Controller
 * (EXCC) et transmise au Canton Controller (CC).
 *
 * Rôle :
 *   - stocker la dernière adresse RailCom reçue
 *   - fournir une API simple :
 *        * begin()     → réinitialise l’adresse
 *        * address()   → retourne l’adresse courante
 *        * onRailcom() → callback appelé lors d’une trame EXCC
 *
 * Ce module ne réalise aucun calcul : il reflète simplement l’information
 * envoyée par EXCC.
 */

#pragma once
#include <stdint.h>

class Railcom
{
public:
    // Initialisation du module RailCom
    static void begin();

    // Adresse RailCom stabilisée reçue d’EXCC
    static uint16_t address();

    // Callback appelé par CC_UartRx lors d’une trame RailCom
    static void onRailcom(uint8_t low, uint8_t high);

private:
    static volatile uint16_t s_address;  // Dernière adresse RailCom reçue
};
