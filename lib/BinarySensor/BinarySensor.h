#ifndef BINARYSENSOR_H
#define BINARYSENSOR_H

#include "Sensor.h"

class BinarySensor : public Sensor
{
    public:
    enum class BinaryState
    {
        Low,
        High
    };

    BinarySensor(int pin) : Sensor(pin)
    {}

    virtual void init() = 0;
    virtual BinaryState readSensor() = 0;
    virtual ~BinarySensor() = default;

    BinaryState getState() const
    {
        return m_state;
    }

    private:
    BinaryState m_state{};
};

#endif