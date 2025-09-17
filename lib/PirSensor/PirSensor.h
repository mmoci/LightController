#ifndef PIRSENSOR_H
#define PIRSENSOR_H

#include "BinarySensor.h"

class PirSensor : public BinarySensor
{
    using OnStateChange = std::function<void(const BinaryState&)>;

    public:
    PirSensor(int pin);

    void init() override;
    void subscribeOnStateChange(OnStateChange onStateChange);
    BinaryState readSensor() override;

    private:
    BinaryState m_state{};
    OnStateChange m_onStateChangeCb{};
};

#endif