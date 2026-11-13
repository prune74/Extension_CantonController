#pragma once
#include <stdint.h>
#include "EXCC_Config.h"

/*
 * Module : EXCC_Calibration
 * Rôle   : Auto‑calibration NON BLOQUANTE du courant à vide.
 *
 * Fonctionnement :
 *   - start()   : lance une calibration (voie supposée vide)
 *   - process() : appelé pour chaque échantillon courant valide reçu à 1 kHz
 *   - isDone()  : indique que les seuils sont prêts
 *
 * API :
 *   - getSeuilLibre()
 *   - getSeuilOccupe()
 *   - setSeuils() (ex : depuis CC)
 */

namespace EXCC_Calibration
{
    void init();

    void start();   // lance une calibration
    void process(uint16_t courant_mA);

    bool isRunning();
    bool isDone();

    uint16_t getSeuilLibre();
    uint16_t getSeuilOccupe();

    void setSeuils(uint16_t libre, uint16_t occupe);
}
