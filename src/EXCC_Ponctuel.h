#pragma once

#include <stdint.h>

class EXCC_Ponctuel
{
public:
    // Initialisation du module
    static void begin();

    // Mise à jour périodique (lecture des deltas + timeout)
    static void loop();

    // Accesseurs pour EXCC_Occupation
    static bool estActifH()  { return s_actifH; }
    static bool estActifAH() { return s_actifAH; }

private:
    // États internes
    static bool s_actifH;
    static bool s_actifAH;

    static unsigned long s_lastH;
    static unsigned long s_lastAH;

    static constexpr unsigned long TIMEOUT_MS = 200;

    // Activation ponctuelle
    static void activerH();
    static void activerAH();

    // Envoi UART vers le CC
    static void envoyerH(bool actif);
    static void envoyerAH(bool actif);
};
