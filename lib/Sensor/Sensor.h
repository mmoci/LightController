#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

class Sensor
{
    public:
    virtual void init() = 0;
    virtual ~Sensor() = default;
};

#endif