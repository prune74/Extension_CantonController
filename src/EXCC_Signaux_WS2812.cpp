#include "EXCC_Signaux_WS2812.h"
#include "EXCC_Config.h"

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

static constexpr uint32_t BLINK_PERIOD_MS = EXCC_WS2812_BLINK_PERIOD_MS;

/* ============================================================================
 *  Constructeur
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
      m_aspectActuel(ASPECT_DEFAUT),
      m_vitesse(ExccVitesse::AUCUNE),
      m_lastBlinkMs(0),
      m_blinkState(true),
      idxRouge(-1),
      idxJaune(-1),
      idxVert(-1),
      idxCarre(-1),
      idxBlanc(-1),
      idxViolet(-1),
      idxRal30_1(-1),
      idxRal30_2(-1),
      idxRal60_1(-1),
      idxRal60_2(-1),
      idxRappel30_1(-1),
      idxRappel30_2(-1),
      idxRappel60_1(-1),
      idxRappel60_2(-1),
      m_hasOeilleton(false),
      m_couleurOeilleton(CRGB::White)
{
    FastLED.setBrightness(EXCC_WS2812_GLOBAL_BRIGHTNESS);
    eteindreTout();
    rebuildMapping();
}

/* ============================================================================
 *  Éteindre toutes les LEDs
 * ==========================================================================*/
void EXCC_Signaux_WS2812::eteindreTout() noexcept
{
    for (uint8_t i = 0; i < m_nbFeux; i++)
        m_feuxStrip[i] = CRGB::Black;

    for (uint8_t i = 0; i < m_nbOeil; i++)
        m_oeilStrip[i] = CRGB::Black;
}

/* ============================================================================
 *  Allumer/éteindre une LED
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
 *  Définir le type de mât
 * ==========================================================================*/
void EXCC_Signaux_WS2812::setType(uint8_t typeSNCF) noexcept
{
    m_typeSNCF = typeSNCF;

    if (m_typeSNCF == SIG_ABSENT)
    {
        eteindreTout();
        m_vitesse = ExccVitesse::AUCUNE;
        m_aspectActuel = ASPECT_DEFAUT;
        return;
    }

    rebuildMapping();
    setAspect(m_aspectActuel);
}

/* ============================================================================
 *  Définir la position
 * ==========================================================================*/
void EXCC_Signaux_WS2812::setPosition(uint8_t pos) noexcept
{
    m_position = pos;
}

/* ============================================================================
 *  Mapping matériel selon le type de mât
 * ==========================================================================*/
void EXCC_Signaux_WS2812::rebuildMapping() noexcept
{
    idxRouge = idxJaune = idxVert = -1;
    idxCarre = idxBlanc = idxViolet = -1;
    idxRal30_1 = idxRal30_2 = -1;
    idxRal60_1 = idxRal60_2 = -1;
    idxRappel30_1 = idxRappel30_2 = -1;
    idxRappel60_1 = idxRappel60_2 = -1;
    m_hasOeilleton = false;

    switch (m_typeSNCF)
    {
    case SIG_BAL: // 3 feux
        idxJaune = 0;
        idxRouge = 1;
        idxVert = 2;
        break;

    case SIG_CARRE: // 5 feux
        idxJaune = 0;
        idxRouge = 1;
        idxVert = 2;
        
        idxCarre = 3;
        m_hasOeilleton = true;
        break;

    case SIG_RAL: // 7 feux (Carré + Œilleton + RAL)
        idxJaune = 0;
        idxRouge = 1;
        idxVert = 2;

        idxCarre = 3;
        m_hasOeilleton = true;

        idxRal30_1 = 4;
        idxRal30_2 = 6;
        idxRal60_1 = 4;
        idxRal60_2 = 6;
        break;

    case SIG_RAPPEL: // 9 feux (Carré + Œilleton + RAL + RAPPEL)
        idxJaune = 0;
        idxRouge = 1;
        idxVert = 2;

        idxCarre = 3;
        m_hasOeilleton = true;

        // RAL 30/60
        idxRal30_1 = 4;
        idxRal30_2 = 6;
        idxRal60_1 = 4;
        idxRal60_2 = 6;

        // RAPPEL 30/60
        idxRappel30_1 = 5;
        idxRappel30_2 = 7;
        idxRappel60_1 = 5;
        idxRappel60_2 = 7;
        break;

    case SIG_MANOEUVRE: // 2 feux
        idxBlanc = 0;
        idxViolet = 1;
        break;

    case SIG_ABSENT:
    default:
        return;
    }
}

/* ============================================================================
 *  Définir l’aspect
 * ==========================================================================*/
bool EXCC_Signaux_WS2812::setAspect(ExsaAspect aspect) noexcept
{
    if (m_typeSNCF == SIG_ABSENT)
    {
        eteindreTout();
        m_vitesse = ExccVitesse::AUCUNE;
        m_aspectActuel = ASPECT_DEFAUT;
        return false;
    }

    if (aspect == m_aspectActuel)
        return false;

    m_aspectActuel = aspect;
    m_vitesse = ExccVitesse::AUCUNE;
    eteindreTout();

    switch (aspect)
    {
    case ASPECT_CARRE:
        setLed(idxCarre, true, CRGB::Red);
        setLed(idxRouge, true, CRGB::Red);
        if (m_hasOeilleton && m_nbOeil > 0)
            m_oeilStrip[0] = m_couleurOeilleton;
        break;

    case ASPECT_SEMAPHORE:
        setLed(idxRouge, true, CRGB::Red);
        if (m_hasOeilleton && m_nbOeil > 0)
            m_oeilStrip[0] = m_couleurOeilleton;
        break;

    case ASPECT_AVERTISSEMENT:
        setLed(idxJaune, true, CRGB::Yellow);
        break;

    case ASPECT_RALENTISSEMENT_30:
        m_vitesse = ExccVitesse::RALENT_30;
        break;

    case ASPECT_RALENTISSEMENT_60:
        m_vitesse = ExccVitesse::RALENT_60;
        break;

    case ASPECT_RAPPEL_30:
        m_vitesse = ExccVitesse::RAPPEL_30;
        break;

    case ASPECT_RAPPEL_60:
        m_vitesse = ExccVitesse::RAPPEL_60;
        break;

    case ASPECT_VOIE_LIBRE:
        setLed(idxVert, true, CRGB::Green);
        break;

    case ASPECT_MANOEUVRE:
        setLed(idxBlanc, true, CRGB::White);
        setLed(idxViolet, true, CRGB::Purple);
        break;

    case ASPECT_MASQUE:
        break;

    case ASPECT_DEFAUT:
    default:
        if (m_hasOeilleton && m_nbOeil > 0)
            m_oeilStrip[0] = m_couleurOeilleton;
        m_vitesse = ExccVitesse::RAPPEL_60;
        break;
    }

    appliquerAspectFixe();
    return true;
}

/* ============================================================================
 *  Feux fixes (RAL 30 / RAPPEL 30)
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
 *  Clignotement (RAL 60 / RAPPEL 60 / BAL)
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
            setLed(idxVert, m_blinkState, CRGB::Green);
        break;
    }
}

/* ============================================================================
 *  Update
 * ==========================================================================*/
void EXCC_Signaux_WS2812::update(uint32_t nowMs) noexcept
{
    if (m_typeSNCF == SIG_ABSENT)
        return;

    appliquerClignotement(nowMs);
}
