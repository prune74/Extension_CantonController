#pragma once
#include <stdint.h>

class EXCC_CompteurEssieux
{
public:
    static void begin(uint16_t idCanton);
    static void loop();

private:
    static void detectSens();
    static void detectFinComptage();
    static void detectFinTrain();
    static void envoyerEssieux(uint8_t essieux);

    static uint16_t s_idCanton;

    static bool s_countingH;
    static bool s_countingAH;
    static bool s_finishedFirst;
};
