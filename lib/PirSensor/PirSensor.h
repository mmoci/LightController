#ifndef PIRSENSOR_H
#define PIRSENSOR_H

#include "BinarySensor.h"

class PirSensor : public BinarySensor
{
    public:
    PirSensor(int pin);

    void init() override;
    void update() override;
    SpecificType getSpecificType() const override {return Sensor::SpecificType::PirSensor;}
};

#endif