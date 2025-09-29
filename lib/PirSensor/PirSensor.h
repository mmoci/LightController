#ifndef PIRSENSOR_H
#define PIRSENSOR_H

#include "BinarySensor.h"

class PirSensor : public BinarySensor
{
    public:
    PirSensor(int pin);

    void init() override;
    BinaryState readSensor() override;
    void bindToLight(Light& light) override;
};

#endif