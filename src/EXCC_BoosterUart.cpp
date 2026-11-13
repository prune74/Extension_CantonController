#include "EXCC_BoosterUart.h"

#include "EXCC_Calibration.h"
#include "EXCC_Pins.h"

#include <Arduino.h>

namespace
{
constexpr uint32_t UART_BAUD = 460800;
constexpr uint32_t HEARTBEAT_PERIOD_MS = 100;
constexpr uint32_t COMMAND_RETRY_MS = 500;
constexpr uint32_t PROFILE_REFRESH_MS = 5000;
constexpr uint32_t SAMPLE_TIMEOUT_MS = 10;
constexpr uint32_t RAILCOM_RECENT_MS = 500;

constexpr uint8_t ACK_OK = 0;

uint16_t readU16Le(const uint8_t *data)
{
    return static_cast<uint16_t>(data[0]) |
           (static_cast<uint16_t>(data[1]) << 8);
}
}

void EXCC_BoosterUart::begin()
{
    Serial2.begin(UART_BAUD, SERIAL_8N1, PIN_BOOSTER_UART_RX, PIN_BOOSTER_UART_TX);
    const uint32_t now = millis();
    m_lastHeartbeatMs = now - HEARTBEAT_PERIOD_MS;
    m_lastPowerCommandMs = now - COMMAND_RETRY_MS;
    m_lastProfileCommandMs = now - COMMAND_RETRY_MS;
}

void EXCC_BoosterUart::process()
{
    receive();

    const uint32_t now = millis();
    if (now - m_lastHeartbeatMs >= HEARTBEAT_PERIOD_MS)
        sendHeartbeat(now);
    if (now - m_lastPowerCommandMs >= COMMAND_RETRY_MS)
        sendPowerCommand(now);
    if (m_profilePending && now - m_lastProfileCommandMs >= COMMAND_RETRY_MS)
        sendProfileCommand(now);
    else if (!m_profilePending &&
             now - m_lastProfileCommandMs >= PROFILE_REFRESH_MS)
    {
        m_profilePending = true;
        sendProfileCommand(now);
    }
}

void EXCC_BoosterUart::setPowerEnabled(bool enabled)
{
    if (m_powerEnabled == enabled)
        return;
    m_powerEnabled = enabled;
    m_lastPowerCommandMs = millis() - COMMAND_RETRY_MS;
}

void EXCC_BoosterUart::setTrackProfile(bool ho)
{
    m_trackProfileHo = ho;
    m_profilePending = true;
    m_lastProfileCommandMs = millis() - COMMAND_RETRY_MS;
}

bool EXCC_BoosterUart::isSampleFresh() const
{
    return m_hasSample && millis() - m_lastSampleMs <= SAMPLE_TIMEOUT_MS;
}

bool EXCC_BoosterUart::isCurrentValid() const
{
    return isSampleFresh() && (m_sampleFlags & 0x01U) != 0;
}

uint16_t EXCC_BoosterUart::current_mA() const { return m_current_mA; }
uint16_t EXCC_BoosterUart::voltage_mV() const { return m_voltage_mV; }
ExccRemoteBoosterState EXCC_BoosterUart::state() const { return m_state; }
bool EXCC_BoosterUart::powerRequested() const { return m_powerEnabled; }
uint16_t EXCC_BoosterUart::railcomAddress() const { return m_railcomAddress; }
uint8_t EXCC_BoosterUart::railcomType() const { return m_railcomType; }
bool EXCC_BoosterUart::hasRecentRailCom() const
{
    return m_lastRailComMs != 0 && millis() - m_lastRailComMs <= RAILCOM_RECENT_MS;
}

bool EXCC_BoosterUart::takeRailComEvent(uint16_t &address, uint8_t &type)
{
    if (!m_railcomPending)
        return false;
    m_railcomPending = false;
    address = m_railcomAddress;
    type = m_railcomType;
    return true;
}

void EXCC_BoosterUart::receive()
{
    BoosterUart::Frame frame;
    while (Serial2.available() > 0)
    {
        if (m_parser.feed(static_cast<uint8_t>(Serial2.read()), frame))
            handleFrame(frame);
    }
}

void EXCC_BoosterUart::handleFrame(const BoosterUart::Frame &frame)
{
    switch (static_cast<BoosterUart::Type>(frame.type))
    {
    case BoosterUart::Type::SAMPLE:
        if (frame.length != 7)
            return;
        m_current_mA = readU16Le(frame.payload);
        m_voltage_mV = readU16Le(frame.payload + 2);
        m_state = static_cast<ExccRemoteBoosterState>(frame.payload[4]);
        m_sampleFlags = frame.payload[6];
        m_lastSampleMs = millis();
        m_hasSample = true;
        if ((m_sampleFlags & 0x01U) != 0)
            EXCC_Calibration::process(m_current_mA);
        break;

    case BoosterUart::Type::RAILCOM:
        if (frame.length != 3)
            return;
        m_railcomType = frame.payload[0];
        m_railcomAddress = readU16Le(frame.payload + 1);
        m_lastRailComMs = millis();
        m_railcomPending = m_railcomAddress != 0;
        break;

    case BoosterUart::Type::ACK:
        if (frame.length == 4 &&
            frame.payload[0] == static_cast<uint8_t>(BoosterUart::Type::SET_PROFILE) &&
            readU16Le(frame.payload + 1) == m_profileSequence &&
            frame.payload[3] == ACK_OK)
            m_profilePending = false;
        break;

    default:
        break;
    }
}

void EXCC_BoosterUart::sendHeartbeat(uint32_t now)
{
    BoosterUart::Frame frame;
    frame.type = static_cast<uint8_t>(BoosterUart::Type::HEARTBEAT);
    frame.sequence = m_txSequence++;
    if (sendFrame(frame))
        m_lastHeartbeatMs = now;
}

void EXCC_BoosterUart::sendPowerCommand(uint32_t now)
{
    BoosterUart::Frame frame;
    frame.type = static_cast<uint8_t>(BoosterUart::Type::SET_POWER);
    frame.sequence = m_txSequence++;
    frame.length = 1;
    frame.payload[0] = m_powerEnabled ? 1 : 0;
    if (sendFrame(frame))
        m_lastPowerCommandMs = now;
}

void EXCC_BoosterUart::sendProfileCommand(uint32_t now)
{
    BoosterUart::Frame frame;
    frame.type = static_cast<uint8_t>(BoosterUart::Type::SET_PROFILE);
    frame.sequence = m_profileSequence = m_txSequence++;
    frame.length = 1;
    frame.payload[0] = m_trackProfileHo ? 1 : 0;
    if (sendFrame(frame))
        m_lastProfileCommandMs = now;
}

bool EXCC_BoosterUart::sendFrame(const BoosterUart::Frame &frame)
{
    uint8_t buffer[BoosterUart::MAX_FRAME_SIZE];
    const size_t length = BoosterUart::encode(frame, buffer, sizeof(buffer));
    if (length == 0 || Serial2.availableForWrite() < static_cast<int>(length))
        return false;
    return Serial2.write(buffer, length) == length;
}
