#include "EXCC_Signaux_WS2812.h"
#include "EXCC_Config.h"

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

static constexpr uint32_t BLINK_PERIOD_MS = EXCC_WS2812_BLINK_PERIOD_MS;

/* ============================================================================
 *  1) Constructeur
 * ==========================================================================*/
EXCC_Signaux_WS2812::EXCC_Signaux_WS2812(CRGB *feux, uint8_t nbFeux,
                                         CRGB *oeilleton, uint8_t nbOeil,
                                         ExccSignalLayout layoutInit) noexcept
    : m_feuxStrip(feux),
      m_nbFeux(nbFeux),
      m_oeilStrip(oeilleton),
      m_nbOeil(nbOeil),
      m_typeSNCF(SIG_ABSENT),
      m_position(0),
      m_layout(layoutInit),
      m_aspectActuel(ExccAspect::ASPECT_MASQUE),
      m_vitesse(ExccVitesse::AUCUNE),
      m_lastBlinkMs(0),
      m_blinkState(true),
      idxRouge(-1), idxJaune(-1),
      idxVoieLibre(-1),
      idxCarre(-1),
      idxRal30_1(-1), idxRal30_2(-1),
      idxRal60_1(-1), idxRal60_2(-1),
      idxRappel30_1(-1), idxRappel30_2(-1),
      idxRappel60_1(-1), idxRappel60_2(-1),
      m_hasOeilleton(false),
      m_couleurOeilleton(CRGB::White)
{
    FastLED.setBrightness(EXCC_WS2812_GLOBAL_BRIGHTNESS);
    eteindreTout();
    rebuildMapping();
}

/* ============================================================================
 *  2) Éteindre toutes les LEDs
 * ==========================================================================*/
void EXCC_Signaux_WS2812::eteindreTout() noexcept
{
    for (uint8_t i = 0; i < m_nbFeux; i++)
        m_feuxStrip[i] = CRGB::Black;

    for (uint8_t i = 0; i < m_nbOeil; i++)
        m_oeilStrip[i] = CRGB::Black;
}

/* ============================================================================
 *  3) Allumer/éteindre une LED
 * ==========================================================================*/
void EXCC_Signaux_WS2812::setLed(int8_t index, bool on, const CRGB &color) noexcept
{
    if (index < 0)
        return;

    uint8_t idx = static_cast<uint8_t>(index);
    if (idx >= m_nbFeux)
        return;

    m_feuxStrip[idx] = on ? color : CRGB::Black;
}

/* ============================================================================
 *  4) Définir le type de mât
 * ==========================================================================*/
void EXCC_Signaux_WS2812::setType(uint8_t typeSNCF) noexcept
{
    m_typeSNCF = typeSNCF;

    if (m_typeSNCF == SIG_ABSENT)
    {
        eteindreTout();
        m_aspectActuel = ExccAspect::ASPECT_MASQUE;
        m_vitesse = ExccVitesse::AUCUNE;
        return;
    }

    rebuildMapping();
    setAspect(m_aspectActuel);
}

/* ============================================================================
 *  5) Définir la position
 * ==========================================================================*/
void EXCC_Signaux_WS2812::setPosition(uint8_t pos) noexcept
{
    m_position = pos;
}

/* ============================================================================
 *  6) Mapping matériel universel Discovery 2026
 * ==========================================================================*/
void EXCC_Signaux_WS2812::rebuildMapping() noexcept
{
    idxRouge = idxJaune = -1;
    idxVoieLibre = -1;
    idxCarre = -1;

    idxRal30_1 = idxRal30_2 = -1;
    idxRal60_1 = idxRal60_2 = -1;
    idxRappel30_1 = idxRappel30_2 = -1;
    idxRappel60_1 = idxRappel60_2 = -1;

    m_hasOeilleton = false;

    switch (m_typeSNCF)
    {
    case SIG_BAL:
        idxJaune     = 0;
        idxRouge     = 1;
        idxVoieLibre = 2;
        break;

    case SIG_CARRE:
        idxJaune     = 0;
        idxRouge     = 1;
        idxVoieLibre = 2;
        idxCarre     = 3;
        m_hasOeilleton = true;
        break;

    case SIG_RAL:
        idxJaune     = 0;
        idxRouge     = 1;
        idxVoieLibre = 2;
        idxCarre     = 3;
        m_hasOeilleton = true;

        idxRal30_1 = 4;
        idxRal30_2 = 5;
        idxRal60_1 = 4;
        idxRal60_2 = 5;
        break;

    case SIG_RAPPEL:
        idxJaune     = 0;
        idxRouge     = 1;
        idxVoieLibre = 2;
        idxCarre     = 3;
        m_hasOeilleton = true;

        idxRal30_1 = 4;
        idxRal30_2 = 6;
        idxRal60_1 = 4;
        idxRal60_2 = 6;

        idxRappel30_1 = 5;
        idxRappel30_2 = 7;
        idxRappel60_1 = 5;
        idxRappel60_2 = 7;
        break;

    case SIG_MANOEUVRE:
        idxVoieLibre = 0;
        idxCarre     = 1;
        break;

    case SIG_ABSENT:
    default:
        return;
    }
}

/* ============================================================================
 *  7) Définir l’aspect (enum class ExccAspect)
 * ==========================================================================*/
bool EXCC_Signaux_WS2812::setAspect(ExccAspect aspect) noexcept
{
    if (m_typeSNCF == SIG_ABSENT)
    {
        eteindreTout();
        m_aspectActuel = ExccAspect::ASPECT_MASQUE;
        m_vitesse = ExccVitesse::AUCUNE;
        return false;
    }

    if (aspect == m_aspectActuel)
        return false;

    m_aspectActuel = aspect;
    m_vitesse = ExccVitesse::AUCUNE;
    eteindreTout();

    switch (aspect)
    {
    case ExccAspect::ASPECT_CARRE:
        setLed(idxCarre, true, CRGB::Red);
        if (m_hasOeilleton) m_oeilStrip[0] = m_couleurOeilleton;
        break;

    case ExccAspect::ASPECT_CARRE_VIOLET:
        setLed(idxCarre, true, CRGB::Purple);
        break;

    case ExccAspect::ASPECT_SEMAPHORE:
        setLed(idxRouge, true, CRGB::Red);
        if (m_hasOeilleton) m_oeilStrip[0] = m_couleurOeilleton;
        break;

    case ExccAspect::ASPECT_AVERTISSEMENT:
        setLed(idxJaune, true, CRGB::Yellow);
        break;

    case ExccAspect::ASPECT_VOIE_LIBRE:
        setLed(idxVoieLibre, true, CRGB::Green);
        break;

    case ExccAspect::ASPECT_MANOEUVRE:
        setLed(idxVoieLibre, true, CRGB::White);
        break;

    case ExccAspect::ASPECT_RALENTISSEMENT_30:
        m_vitesse = ExccVitesse::RALENT_30;
        break;

    case ExccAspect::ASPECT_RALENTISSEMENT_60:
        m_vitesse = ExccVitesse::RALENT_60;
        break;

    case ExccAspect::ASPECT_RAPPEL_30:
        m_vitesse = ExccVitesse::RAPPEL_30;
        break;

    case ExccAspect::ASPECT_RAPPEL_60:
        m_vitesse = ExccVitesse::RAPPEL_60;
        break;

    case ExccAspect::ASPECT_MASQUE:
    default:
        break;
    }

    appliquerAspectFixe();
    return true;
}

/* ============================================================================
 *  8) Feux fixes (RAL 30 / RAPPEL 30)
 * ==========================================================================*/
void EXCC_Signaux_WS2812::appliquerAspectFixe() noexcept
{
    switch (m_vitesse)
    {
    case ExccVitesse::RALENT_30:
        setLed(idxRal30_1, true, CRGB::Yellow);
        setLed(idxRal30_2, true, CRGB::Yellow);
        break;

    case ExccVitesse::RAPPEL_30:
        setLed(idxRappel30_1, true, CRGB::Yellow);
        setLed(idxRappel30_2, true, CRGB::Yellow);
        break;

    default:
        break;
    }
}

/* ============================================================================
 *  9) Clignotement (RAL 60 / RAPPEL 60 / BAL vert)
 * ==========================================================================*/
void EXCC_Signaux_WS2812::appliquerClignotement(uint32_t nowMs) noexcept
{
    if (m_typeSNCF == SIG_ABSENT)
        return;

    if (m_vitesse == ExccVitesse::AUCUNE && m_typeSNCF != SIG_BAL)
        return;

    if (nowMs - m_lastBlinkMs >= BLINK_PERIOD_MS)
    {
        m_lastBlinkMs = nowMs;
        m_blinkState = !m_blinkState;
    }

    switch (m_vitesse)
    {
    case ExccVitesse::RALENT_60:
        setLed(idxRal60_1, m_blinkState, CRGB::Yellow);
        setLed(idxRal60_2, m_blinkState, CRGB::Yellow);
        break;

    case ExccVitesse::RAPPEL_60:
        setLed(idxRappel60_1, m_blinkState, CRGB::Yellow);
        setLed(idxRappel60_2, m_blinkState, CRGB::Yellow);
        break;

    default:
        if (m_typeSNCF == SIG_BAL)
            setLed(idxVoieLibre, m_blinkState, CRGB::Green);
        break;
    }
}

/* ============================================================================
 *  🔟 Update
 * ==========================================================================*/
void EXCC_Signaux_WS2812::update(uint32_t nowMs) noexcept
{
    if (m_typeSNCF == SIG_ABSENT)
        return;

    appliquerClignotement(nowMs);
}
