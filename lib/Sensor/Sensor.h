#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

class Sensor
{
    protected:
    int m_pin{};
    
    public:
    Sensor(int pin) : m_pin{pin}
    {}
    
    virtual void init() = 0;
    virtual ~Sensor() = default;
};

#endif