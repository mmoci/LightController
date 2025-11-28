#include "BH1750.h"

BH1750::BH1750(uint16_t deviceAddress, Mode mode) : SensorI2C(deviceAddress), m_mode{mode}
{}

void BH1750::init()
{
    m_isConnected = i2cDevicePresent();

    if (!m_isConnected)
    {
        Serial.println("[BHT1750] Device not present!");
        return;
    }

    if(!setState(State::PowerOn)) m_valid = false;
    if(!setMode()) m_valid = false;
}

void BH1750::update()
{
    if (!m_isConnected)
        return;

    switch(m_mode)
    {
        case Mode::ContinuousHighRes:
        case Mode::ContinuousHighRes2:
        case Mode::ContinuousLowRes:
            readContinuous();
        break;

        default:
        Serial.println("[BHT1750] Not supported mode for update() function!");
    }
}

void BH1750::readContinuous()
{
    if(millis() - m_lastReadMs < measurementTimeMs(m_mode))
        return;

    m_lastReadMs = millis();

    std::uint8_t buffer[NR_OF_BYTES]{};
    if(!readBytes(buffer, NR_OF_BYTES))
    {
        m_valid = false;
        return;
    }

    m_valid = true;
    updateLuminance(buffer, NR_OF_BYTES);
}

void BH1750::readOneTime()
{
    std::uint8_t buffer[NR_OF_BYTES]{};

    // In One Time mode you must send mode command every time you want to read as it gooes to sleep after reading.
    uint8_t mode = static_cast<uint8_t>(m_mode);
    writeBytes(&mode, 1);
    delay(measurementTimeMs(m_mode));

    if(!readBytes(buffer, NR_OF_BYTES))
    {
        m_valid = false;
        return;
    }

    m_valid = true;
    updateLuminance(buffer, NR_OF_BYTES);
}

void BH1750::updateLuminance(const std::uint8_t* buffer, std::size_t nrOfBytes)
{
    std::uint16_t rawValue{};

    for(std::size_t byte{0}; byte < nrOfBytes; ++byte)
    {
        rawValue = (rawValue << 8) | buffer[byte];
    }

    switch(m_mode)
    {
        case Mode::ContinuousHighRes:
        case Mode::OneTimeHighRes:
            m_luminance = rawValue / 1.2;
            break;

        case Mode::ContinuousHighRes2:
        case Mode::OneTimeHighRes2:
            m_luminance = rawValue / 2.4;
            break;
        
        case Mode::ContinuousLowRes:
        case Mode::OneTimeLowRes:
            m_luminance = (rawValue / 1.2) * 4;
            break;
            
        default:
            Serial.println("[BHT1750] Invalid mode!");
    }
}

constexpr uint16_t BH1750::measurementTimeMs(Mode mode)
{
    switch (mode)
    {
        case Mode::OneTimeLowRes:
        case Mode::ContinuousLowRes:
            return 24; //ms
        case Mode::OneTimeHighRes:
        case Mode::ContinuousHighRes:
        case Mode::OneTimeHighRes2:
        case Mode::ContinuousHighRes2:
        default:
            return 180; //ms
    }
}

bool BH1750::setState(State state)
{
    int errorCode = writeBytes(reinterpret_cast<const std::uint8_t*>(&state), 1);  // Send POWER_ON command
    if(errorCode)
    {
        Serial.print("[BH1750] Failed to power on device, error when writing, errorCode: ");
        Serial.println(errorCode);
        return false;
    }
    return true;
}

bool BH1750::setMode()
{
    uint8_t mode{static_cast<uint8_t>(m_mode)};
    uint8_t errorCode = writeBytes(&mode, 1); // Send mode command
    if(errorCode)
    {
        Serial.println("[BH1750] Failed to set mode on device, error when writing, errorCode: ");
        Serial.println(errorCode);
        return false;
    }
    return true;
}

bool BH1750::isLowLight()
{
    return static_cast<uint16_t>(std::round(m_luminance)) < LUMINANCE_THRESHOLD;
}