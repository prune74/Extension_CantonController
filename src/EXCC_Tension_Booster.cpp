#include "EXCC_Tension_Booster.h"
#include <Wire.h>
#include "EXCC_Pins.h"
#include <Arduino.h>

// Adresse I2C du digipot (à ajuster selon ton câblage)
static constexpr uint8_t DIGIPOT_ADDR = 0x2E;

// Registre du wiper (MCP4551 / MCP4561)
static constexpr uint8_t DIGIPOT_REG  = 0x00;

// Valeurs à ajuster après calibration
static constexpr uint8_t DIGIPOT_HO = 23;   // 15V
static constexpr uint8_t DIGIPOT_N  = 30;   // 12V

void EXCC_Tension_Booster::begin() noexcept
{
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
}

void EXCC_Tension_Booster::setTrackProfile(bool ho) noexcept
{
#if EXCC_DEBUG
    Serial.printf("[EXCC] PROFILE_VOIE → %s\n", ho ? "HO (15V)" : "N (12V)");
#endif

    uint8_t value = ho ? DIGIPOT_HO : DIGIPOT_N;
    writeDigipot(value);
}

void EXCC_Tension_Booster::writeDigipot(uint8_t value) noexcept
{
    Wire.beginTransmission(DIGIPOT_ADDR);
    Wire.write(DIGIPOT_REG);
    Wire.write(value);
    Wire.endTransmission();
}
