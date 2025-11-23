#ifndef BH1750_H
#define BH1750_H

#include "SensorI2C.h"

/*
BH1750 has several modes (H-resolution, L-resolution, continuous, one-shot).

High-Resolution Mode (1 lx precision, 120ms), 
High-Resolution Mode 2 (0.5 lx precision, 120ms)
and Low-Resolution Mode (4 lx precision, 16ms).
Continuous: device keeps measuring, you can requestFrom() whenever you want (possibly no extra write); better for periodic sampling.
One-shot: you send a measurement command, wait the conversion time, then read — good for infrequent readings and power saving.

BH1750 returns a small, fixed number of bytes (so it fits your API). 
You’ll need a short measurement delay in one-shot mode; continuous mode may need no delay or a much smaller one.
*/

class BH1750 : public SensorI2C
{
    public:
    enum class State : uint8_t
    {
        PowerDown = 0x00,
        PowerOn = 0x01,
        Reset = 0x07
    };

    enum class Mode : uint8_t 
    { 
        ContinuousHighRes = 0x10,
        ContinuousHighRes2 = 0x11,
        ContinuousLowRes = 0x13, 
        OneTimeHighRes = 0x20,
        OneTimeHighRes2 = 0x21,
        OneTimeLowRes = 0x23
    };

    BH1750(uint16_t deviceAddress, Mode mode);

    void init() override;
    void update() override;
    float getLuminance() const {return m_luminance;}
    bool isValid() const { return m_valid; }
    bool setState(State state); // POWER ON/OFF/RESET
    bool setReset();
    bool setMode();

    private:
    inline static constexpr std::size_t NR_OF_BYTES {2};
    Mode m_mode{};
    float m_luminance{};
    unsigned long m_lastReadMs{};
    bool m_valid{false};

    void readContinuous();
    void readOneTime();
    void updateLuminance(const std::uint8_t* buffer, std::size_t nrOfBytes);
    static constexpr uint16_t measurementTimeMs(Mode mode);
};

#endif