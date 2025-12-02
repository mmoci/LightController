#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

class Sensor
{
    public:
    enum class SpecificType
    {
        Button,
        PirSensor,
        BH1750
    };

    virtual void init() = 0;
    virtual void update() = 0;
    virtual SpecificType getSpecificType() const = 0;
    virtual ~Sensor() = default;
};

#endif