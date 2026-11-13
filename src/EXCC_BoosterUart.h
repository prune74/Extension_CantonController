#pragma once

#include "UartProtocol.h"

#include <stdint.h>

enum class ExccRemoteBoosterState : uint8_t
{
    OK = 0,
    FAULT = 1,
    OVERHEAT = 2,
    OFF = 3,
    RETRY = 4
};

class EXCC_BoosterUart
{
public:
    void begin();
    void process();
    void setPowerEnabled(bool enabled);
    void setTrackProfile(bool ho);

    bool isSampleFresh() const;
    bool isCurrentValid() const;
    uint16_t current_mA() const;
    uint16_t voltage_mV() const;
    ExccRemoteBoosterState state() const;
    bool powerRequested() const;
    uint16_t railcomAddress() const;
    uint8_t railcomType() const;
    bool hasRecentRailCom() const;
    bool takeRailComEvent(uint16_t &address, uint8_t &type);

private:
    void receive();
    void handleFrame(const BoosterUart::Frame &frame);
    void sendHeartbeat(uint32_t now);
    void sendPowerCommand(uint32_t now);
    void sendProfileCommand(uint32_t now);
    bool sendFrame(const BoosterUart::Frame &frame);

    BoosterUart::Parser m_parser;
    uint16_t m_txSequence = 0;
    uint16_t m_profileSequence = 0;
    uint32_t m_lastHeartbeatMs = 0;
    uint32_t m_lastPowerCommandMs = 0;
    uint32_t m_lastProfileCommandMs = 0;
    uint32_t m_lastSampleMs = 0;
    uint32_t m_lastRailComMs = 0;
    uint16_t m_current_mA = 0;
    uint16_t m_voltage_mV = 0;
    uint16_t m_railcomAddress = 0;
    uint8_t m_railcomType = 0;
    uint8_t m_sampleFlags = 0;
    ExccRemoteBoosterState m_state = ExccRemoteBoosterState::OFF;
    bool m_powerEnabled = true;
    bool m_trackProfileHo = false;
    bool m_hasSample = false;
    bool m_profilePending = true;
    bool m_railcomPending = false;
};
