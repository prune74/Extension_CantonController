#pragma once

#include <stdint.h>

class EXCC_Ponctuel
{
public:
    static void begin();
    static void loop();

private:
    static bool s_actifH;
    static bool s_actifAH;

    static unsigned long s_lastH;
    static unsigned long s_lastAH;

    static constexpr unsigned long TIMEOUT_MS = 200;

    static void activerH();
    static void activerAH();

    static void envoyerH(bool actif);
    static void envoyerAH(bool actif);
};
