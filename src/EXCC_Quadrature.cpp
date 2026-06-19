#include "EXCC_Quadrature.h"
#include <Arduino.h>

/*
 * ============================================================
 *  Pointeurs internes pour les ISR
 * ============================================================
 */
static EXCC_Quadrature *instanceH  = nullptr;
static EXCC_Quadrature *instanceAH = nullptr;

/*
 * ============================================================
 *  ISR externes — redirigent vers l’instance correspondante
 * ============================================================
 */
void IRAM_ATTR isrQuadH()
{
    if (instanceH)
        instanceH->isr();
}

void IRAM_ATTR isrQuadAH()
{
    if (instanceAH)
        instanceAH->isr();
}

/*
 * ============================================================
 *  Constructeur
 * ============================================================
 */
EXCC_Quadrature::EXCC_Quadrature()
    : m_pinA(GPIO_NUM_NC),
      m_pinB(GPIO_NUM_NC),
      m_queue(nullptr)
{
}

/*
 * ============================================================
 *  begin() — initialisation des pins et de la queue
 * ============================================================
 */
void EXCC_Quadrature::begin(gpio_num_t pinA, gpio_num_t pinB)
{
    m_pinA = pinA;
    m_pinB = pinB;

    initQueue();
}

/*
 * ============================================================
 *  initQueue() — création de la queue FreeRTOS
 * ============================================================
 */
void EXCC_Quadrature::initQueue()
{
    m_queue = xQueueCreate(32, sizeof(uint8_t));
}

/*
 * ============================================================
 *  installerInterruptions() — attache les ISR
 * ============================================================
 */
void EXCC_Quadrature::installerInterruptions()
{
    pinMode(m_pinA, INPUT);
    pinMode(m_pinB, INPUT);

    // Déterminer si cette instance est H ou AH
    if (this == &quadH)
    {
        instanceH = this;
        attachInterrupt(m_pinA, isrQuadH, CHANGE);
        attachInterrupt(m_pinB, isrQuadH, CHANGE);
    }
    else if (this == &quadAH)
    {
        instanceAH = this;
        attachInterrupt(m_pinA, isrQuadAH, CHANGE);
        attachInterrupt(m_pinB, isrQuadAH, CHANGE);
    }
}

/*
 * ============================================================
 *  isr() — ISR interne ultra courte
 * ============================================================
 */
void IRAM_ATTR EXCC_Quadrature::isr()
{
    uint8_t a = digitalRead(m_pinA);
    uint8_t b = digitalRead(m_pinB);
    uint8_t etat = (a << 1) | b;

    BaseType_t hp = pdFALSE;
    xQueueSendFromISR(m_queue, &etat, &hp);

    if (hp)
        portYIELD_FROM_ISR();
}

/*
 * ============================================================
 *  lireEvenement() — lecture non bloquante
 * ============================================================
 */
bool EXCC_Quadrature::lireEvenement(uint8_t &etat)
{
    return xQueueReceive(m_queue, &etat, 0) == pdTRUE;
}
