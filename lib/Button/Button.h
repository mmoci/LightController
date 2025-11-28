#ifndef BUTTON_H
#define BUTTON_H

#include "BinarySensor.h"

class Button : public BinarySensor
{
    public:
    Button(int pin);

    void init() override;
    void update() override;
    SpecificType getSpecificType() const override {return Sensor::SpecificType::Button;}

    private:
    BinaryState m_lastState{BinaryState::Low};
    BinaryState m_stableState{BinaryState::Low};
    unsigned long m_lastChangeTime{};
    static constexpr unsigned long DEBOUNCE_DELAY {50}; // ms
};

#endif