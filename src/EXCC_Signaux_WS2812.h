#pragma once

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

#include <Arduino.h>
#include "WsCrgb.h"
#include "Protocol.h"

// Layout physique éventuel (orientation, inversion…)
enum class ExccSignalLayout : uint8_t
{
    NORMAL = 0,
    MANOEUVRE = 1
};

// États internes pour les ralentissements / rappels
enum class ExccVitesse : uint8_t
{
    AUCUNE = 0,
    RALENT_30,
    RALENT_60,
    RAPPEL_30,
    RAPPEL_60
};

class EXCC_Signaux_WS2812
{
public:
    EXCC_Signaux_WS2812(WsCrgb *feux, uint8_t nbFeux,
                        WsCrgb *oeilleton, uint8_t nbOeil,
                        ExccSignalLayout layoutInit) noexcept;

    // Commandes principales
    bool setAspect(ExccAspect aspect) noexcept;
    void update(uint32_t nowMs) noexcept;

    // Configuration matérielle
    void setType(uint8_t typeSNCF) noexcept;
    void setLayout(ExccSignalLayout layout) noexcept { m_layout = layout; }
    void setPosition(uint8_t pos) noexcept;

    // Accesseurs
    ExccAspect getAspect() const noexcept { return m_aspectActuel; }
    uint8_t getType() const noexcept { return m_typeSNCF; }
    uint8_t getPosition() const noexcept { return m_position; }

private:
    // ---------------------------------------------------------------------
    // Données LED
    // ---------------------------------------------------------------------
    WsCrgb *m_feuxStrip;
    uint8_t m_nbFeux;

    WsCrgb *m_oeilStrip;
    uint8_t m_nbOeil;

    // ---------------------------------------------------------------------
    // Configuration matérielle
    // ---------------------------------------------------------------------
    uint8_t m_typeSNCF;        // SignalProfil (TYPE_A, TYPE_C…)
    uint8_t m_position;        // Position physique éventuelle
    ExccSignalLayout m_layout; // Layout physique

    // ---------------------------------------------------------------------
    // État courant
    // ---------------------------------------------------------------------
    ExccAspect m_aspectActuel;
    ExccVitesse m_vitesse;

    uint32_t m_lastBlinkMs;
    bool m_blinkState;

    // ---------------------------------------------------------------------
    // Mapping interne des indices (dépend du type de mât)
    // ---------------------------------------------------------------------
    int8_t idxRouge;
    int8_t idxJaune;
    int8_t idxVoieLibre; // VERT / BLANC
    int8_t idxCarre;     // ROUGE / VIOLET

    int8_t idxRal30_1;
    int8_t idxRal30_2;
    int8_t idxRal60_1;
    int8_t idxRal60_2;

    int8_t idxRappel30_1;
    int8_t idxRappel30_2;
    int8_t idxRappel60_1;
    int8_t idxRappel60_2;

    bool m_hasOeilleton;
    WsCrgb m_couleurOeilleton;

    // ---------------------------------------------------------------------
    // Méthodes internes
    // ---------------------------------------------------------------------
    void eteindreTout() noexcept;
    void setLed(int8_t index, bool on, const WsCrgb &color) noexcept;

    void rebuildMapping() noexcept;
    void appliquerAspectFixe() noexcept;
    void appliquerClignotement(uint32_t nowMs) noexcept;
};
