#ifndef BUTTON_H
#define BUTTON_H

#include "BinarySensor.h"

class Button : public BinarySensor
{
    public:
    Button(int pin);

    void init() override;
    BinaryState readSensor() override;
    void bindToLight(std::shared_ptr<Light> lightPtr) override;

    private:
    BinaryState m_lastState{BinaryState::Low};
    BinaryState m_stableState{BinaryState::Low};
    unsigned long m_lastChangeTime{};
    static constexpr unsigned long DEBOUNCE_DELAY {50}; // ms
};

#endif