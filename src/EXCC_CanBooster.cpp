#include "EXCC_CanBooster.h"
#include "EXCC_CanConfig.h"
#include "CanInit.h"
#include "CanBus.h"
#include "EXCC_Booster_WS2812.h"
#include "EXCC_BoosterCore.h"

#include <Arduino.h>
#include <esp_err.h>
#include <esp_timer.h>

#ifndef ESP32
#error "Ce module doit être compilé pour ESP32 uniquement."
#endif

extern EXCC_Booster_WS2812 booster;

namespace
{
constexpr size_t DCC_FIFO_CAPACITY = 256;
constexpr uint8_t DCC_MAX_BITS_PER_FRAME = 14; // DLC CAN <= 8 octets

static EXCC_CanConfig canConfig;
static uint8_t s_dccFifo[DCC_FIFO_CAPACITY];
static size_t s_dccHead = 0;
static size_t s_dccTail = 0;
static size_t s_dccCount = 0;
static volatile bool s_cutoutActive = false;
static bool s_cutoutPending = false;
static size_t s_cutoutBitsRemaining = 0;
static volatile bool s_cutoutExpired = false;
static esp_timer_handle_t s_cutoutTimer = nullptr;

static uint32_t s_lastCanRxMs = 0;
static uint32_t s_rxCount = 0;
static uint32_t s_lastCutoutMs = 0;
static uint32_t s_rejectedDccBlocks = 0;
static uint32_t s_lastRejectLogMs = 0;

void cutoutTimerCallback(void *)
{
    s_cutoutExpired = true;
    EXCC_BoosterCore::notifyCutoutTimer();
}

void finishCutoutIfExpired()
{
    if (!s_cutoutActive || !s_cutoutExpired)
        return;

    s_cutoutExpired = false;
    s_cutoutActive = false;
    booster.onCutoutEnd();
}

void rejectDccFrame(const char *reason)
{
#if EXCC_DEBUG
    ++s_rejectedDccBlocks;
    const uint32_t now = millis();
    if (s_lastRejectLogMs == 0 || now - s_lastRejectLogMs >= 1000)
    {
        Serial.printf("[CAN DCC] %lu bloc(s) 0x100 rejetes (derniere raison: %s)\n",
                      static_cast<unsigned long>(s_rejectedDccBlocks), reason);
        s_rejectedDccBlocks = 0;
        s_lastRejectLogMs = now ? now : 1;
    }
#else
    (void)reason;
#endif
}

void handleDccBlock(const CanMsg &msg)
{
    if (msg.dlc < 2 || msg.dlc > sizeof(msg.data))
    {
        rejectDccFrame("DLC invalide");
        return;
    }

    const uint8_t bitCount = msg.data[0];
    if (bitCount == 0 || bitCount > DCC_MAX_BITS_PER_FRAME)
    {
        rejectDccFrame("N doit etre compris entre 1 et 14");
        return;
    }

    const uint8_t expectedDlc = 1 + (bitCount + 1) / 2;
    if (msg.dlc != expectedDlc)
    {
        rejectDccFrame("DLC incompatible avec N");
        return;
    }

    if (DCC_FIFO_CAPACITY - s_dccCount < bitCount)
    {
        rejectDccFrame("FIFO pleine");
        return;
    }

    for (uint8_t i = 0; i < bitCount; ++i)
    {
        const uint8_t packed = msg.data[1 + i / 2];
        const uint8_t nibble = (i & 1U) ? (packed >> 4) : (packed & 0x0F);
        s_dccFifo[s_dccTail] = (nibble >> 1) & 1U;
        s_dccTail = (s_dccTail + 1) % DCC_FIFO_CAPACITY;
    }
    s_dccCount += bitCount;
}

void handleCutout(const CanMsg &msg)
{
    if (msg.dlc < 1)
        return;

    s_lastCutoutMs = millis();

    const bool localCutout = msg.data[0] != 0;
    const bool globalCutout = msg.dlc >= 2 && msg.data[1] != 0;
    const bool shouldStart =
        localCutout || (globalCutout && EXCC_BOOSTER_ENABLE_GLOBAL_CUTOUT);

    // In the converter protocol, false/false is the END marker. The
    // cutout duration is timed locally instead of using CAN frame spacing.
    if (!shouldStart || s_cutoutActive || s_cutoutPending)
        return;

    s_cutoutPending = true;
    s_cutoutBitsRemaining = s_dccCount;
}
} // namespace

void EXCC_CanBooster::begin()
{
    CanInit::begin(canConfig);

    esp_timer_create_args_t timerArgs = {};
    timerArgs.callback = cutoutTimerCallback;
    timerArgs.name = "exccCutout";
    const esp_err_t err = esp_timer_create(&timerArgs, &s_cutoutTimer);
    if (err != ESP_OK)
    {
#if EXCC_DEBUG
        Serial.printf("[RAILCOM] Echec creation timer: %s\n", esp_err_to_name(err));
#endif
        s_cutoutTimer = nullptr;
    }
}

void EXCC_CanBooster::process()
{
    finishCutoutIfExpired();

    CanMsg msg;
    while (CanBus::bus(0).receive(msg))
    {
        ++s_rxCount;
        s_lastCanRxMs = millis();
        handleFrame(msg);
    }

    finishCutoutIfExpired();
}

void EXCC_CanBooster::handleFrame(const CanMsg &msg)
{
    if (msg.id == 0x100)
    {
        handleDccBlock(msg);
        return;
    }

    if (msg.id == 0x101)
    {
        handleCutout(msg);
        return;
    }

    booster.onCanMessage(msg.id, msg.data, msg.dlc);
}

bool EXCC_CanBooster::isCutoutActive()
{
    return s_cutoutActive || s_cutoutPending;
}

bool EXCC_CanBooster::isCutoutPending()
{
    return s_cutoutPending;
}

void EXCC_CanBooster::startPendingCutout()
{
    if (!s_cutoutPending || s_cutoutBitsRemaining != 0 || booster.isDccBusy())
        return;

    s_cutoutPending = false;
    if (s_cutoutTimer == nullptr)
    {
#if EXCC_DEBUG
        Serial.println("[RAILCOM] Cutout ignore: timer indisponible");
#endif
        return;
    }

    s_cutoutExpired = false;
    s_cutoutActive = true;
    booster.onCutoutStart();

    const esp_err_t err = esp_timer_start_once(
        s_cutoutTimer, EXCC_BOOSTER_CUTOUT_US);
    if (err != ESP_OK)
    {
        s_cutoutActive = false;
        booster.onCutoutEnd();
#if EXCC_DEBUG
        Serial.printf("[RAILCOM] Echec demarrage timer: %s\n", esp_err_to_name(err));
#endif
    }
}

uint32_t EXCC_CanBooster::lastCanRxMs()
{
    return s_lastCanRxMs;
}

uint32_t EXCC_CanBooster::lastCutoutMs()
{
    return s_lastCutoutMs;
}

uint32_t EXCC_CanBooster::rxCount()
{
    return s_rxCount;
}

size_t EXCC_CanBooster::peekDccBits(uint8_t *bits, size_t capacity)
{
    if (bits == nullptr || capacity == 0)
        return 0;

    size_t count = (s_dccCount < capacity) ? s_dccCount : capacity;
    if (s_cutoutPending && count > s_cutoutBitsRemaining)
        count = s_cutoutBitsRemaining;
    for (size_t i = 0; i < count; ++i)
        bits[i] = s_dccFifo[(s_dccHead + i) % DCC_FIFO_CAPACITY];
    return count;
}

void EXCC_CanBooster::consumeDccBits(size_t count)
{
    if (count > s_dccCount)
        count = s_dccCount;
    if (s_cutoutPending && count > s_cutoutBitsRemaining)
        count = s_cutoutBitsRemaining;

    s_dccHead = (s_dccHead + count) % DCC_FIFO_CAPACITY;
    s_dccCount -= count;
    if (s_cutoutPending)
        s_cutoutBitsRemaining -= count;
}

void EXCC_CanBooster::discardDccBits()
{
    s_dccHead = s_dccTail;
    s_dccCount = 0;
    s_cutoutPending = false;
    s_cutoutBitsRemaining = 0;
}
