#include "EXCC_Signaux_WS2812.h"
#include "EXCC_Config.h"

static constexpr uint32_t BLINK_PERIOD_MS = EXCC_WS2812_BLINK_PERIOD_MS;

EXCC_Signaux_WS2812::EXCC_Signaux_WS2812(CRGB* feux, uint8_t nbFeux,
                                         CRGB* oeilleton, uint8_t nbOeil,
                                         ExccSignalLayout layoutInit) noexcept
    : m_feuxStrip(feux),
      m_nbFeux(nbFeux),
      m_oeilStrip(oeilleton),
      m_nbOeil(nbOeil),
      m_typeSNCF(SIG_SIMPLE),
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

void EXCC_Signaux_WS2812::eteindreTout() noexcept
{
    for (uint8_t i = 0; i < m_nbFeux; i++)
        m_feuxStrip[i] = CRGB::Black;

    for (uint8_t i = 0; i < m_nbOeil; i++)
        m_oeilStrip[i] = CRGB::Black;
}

void EXCC_Signaux_WS2812::setLed(int8_t index, bool on, const CRGB& color) noexcept
{
    if (index < 0)
        return;
    uint8_t idx = static_cast<uint8_t>(index);
    if (idx >= m_nbFeux)
        return;

    m_feuxStrip[idx] = on ? color : CRGB::Black;
}

void EXCC_Signaux_WS2812::setType(uint8_t typeSNCF) noexcept
{
    m_typeSNCF = typeSNCF;

    // V4.2 : mât absent → tout éteindre et ne rien mapper
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

void EXCC_Signaux_WS2812::setLayout(ExccSignalLayout layout) noexcept
{
    m_layout = layout;

    if (m_typeSNCF == SIG_ABSENT)
        return;

    rebuildMapping();
    setAspect(m_aspectActuel);
}

void EXCC_Signaux_WS2812::setPosition(uint8_t pos) noexcept
{
    m_position = pos;
}

void EXCC_Signaux_WS2812::rebuildMapping() noexcept
{
    idxRouge = idxJaune = idxVert = -1;
    idxCarre = idxBlanc = idxViolet = -1;
    idxRal30_1 = idxRal30_2 = -1;
    idxRal60_1 = idxRal60_2 = -1;
    idxRappel30_1 = idxRappel30_2 = -1;
    idxRappel60_1 = idxRappel60_2 = -1;
    m_hasOeilleton = false;
    m_couleurOeilleton = CRGB::White;

    if (m_typeSNCF == SIG_ABSENT)
        return;

    switch (m_typeSNCF)
    {
    case SIG_SIMPLE:
        idxJaune = 0;
        idxRouge = 1;
        idxVert  = 2;
        break;

    case SIG_CARRE:
        idxJaune = 0;
        idxRouge = 1;
        idxVert  = 2;
        idxCarre = 4;
        m_hasOeilleton = true;
        break;

    case SIG_RAL:
        idxJaune = 0;
        idxRouge = 1;
        idxVert  = 2;
        idxRal30_1 = 5;
        idxRal30_2 = 7;
        idxRal60_1 = 5;
        idxRal60_2 = 7;
        break;

    case SIG_RAPPEL:
        idxJaune = 0;
        idxRouge = 1;
        idxVert  = 2;
        idxRappel30_1 = 6;
        idxRappel30_2 = 8;
        idxRappel60_1 = 6;
        idxRappel60_2 = 8;
        break;

    case SIG_MANOEUVRE:
        idxBlanc  = 0;
        idxViolet = 1;
        break;

    case SIG_BAL:
        idxRouge = 1;
        idxJaune = 0;
        idxVert  = 2;
        break;

    case SIG_ENTREE:
        idxJaune = 0;
        idxRouge = 1;
        idxVert  = 2;
        idxCarre = 4;
        idxRal30_1 = 5;
        idxRal30_2 = 7;
        idxRal60_1 = 5;
        idxRal60_2 = 7;
        m_hasOeilleton = true;
        break;

    case SIG_SORTIE:
        idxJaune = 0;
        idxRouge = 1;
        idxVert  = 2;
        idxRappel30_1 = 6;
        idxRappel30_2 = 8;
        idxRappel60_1 = 6;
        idxRappel60_2 = 8;
        break;

    default:
        idxJaune = 0;
        idxRouge = 1;
        idxVert  = 2;
        break;
    }
}

bool EXCC_Signaux_WS2812::setAspect(ExsaAspect aspect) noexcept
{
    // V4.2 : mât absent → ignorer toute demande d’aspect
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
        if (idxCarre >= 0)
            setLed(idxCarre, true, CRGB::Red);
        if (idxRouge >= 0)
            setLed(idxRouge, true, CRGB::Red);
        if (m_hasOeilleton && m_nbOeil > 0)
            m_oeilStrip[0] = m_couleurOeilleton;
        break;

    case ASPECT_SEMAPHORE:
        if (idxRouge >= 0)
            setLed(idxRouge, true, CRGB::Red);
        if (m_hasOeilleton && m_nbOeil > 0)
            m_oeilStrip[0] = m_couleurOeilleton;
        break;

    case ASPECT_AVERTISSEMENT:
        if (idxJaune >= 0)
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
        if (idxVert >= 0)
            setLed(idxVert, true, CRGB::Green);
        break;

    case ASPECT_MANOEUVRE:
        if (idxBlanc >= 0)
            setLed(idxBlanc, true, CRGB::White);
        if (idxViolet >= 0)
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

void EXCC_Signaux_WS2812::appliquerAspectFixe() noexcept
{
    switch (m_vitesse)
    {
    case ExccVitesse::RALENT_30:
        if (idxRal30_1 >= 0) setLed(idxRal30_1, true, CRGB::Yellow);
        if (idxRal30_2 >= 0) setLed(idxRal30_2, true, CRGB::Yellow);
        break;

    case ExccVitesse::RAPPEL_30:
        if (idxRappel30_1 >= 0) setLed(idxRappel30_1, true, CRGB::Yellow);
        if (idxRappel30_2 >= 0) setLed(idxRappel30_2, true, CRGB::Yellow);
        break;

    default:
        break;
    }
}

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
        if (idxRal60_1 >= 0) setLed(idxRal60_1, m_blinkState, CRGB::Yellow);
        if (idxRal60_2 >= 0) setLed(idxRal60_2, m_blinkState, CRGB::Yellow);
        break;

    case ExccVitesse::RAPPEL_60:
        if (idxRappel60_1 >= 0) setLed(idxRappel60_1, m_blinkState, CRGB::Yellow);
        if (idxRappel60_2 >= 0) setLed(idxRappel60_2, m_blinkState, CRGB::Yellow);
        break;

    default:
        if (m_typeSNCF == SIG_BAL && idxVert >= 0)
            setLed(idxVert, m_blinkState, CRGB::Green);
        break;
    }
}

void EXCC_Signaux_WS2812::update(uint32_t nowMs) noexcept
{
    if (m_typeSNCF == SIG_ABSENT)
        return;

    appliquerClignotement(nowMs);
}
