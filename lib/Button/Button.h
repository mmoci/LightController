#ifndef BUTTON_H
#define BUTTON_H

#include "BinarySensor.h"

class Button : public BinarySensor
{
    using OnStateChange = std::function<void(const BinaryState&)>;

    public:
    Button(int pin);

    void init() override;
    BinaryState readSensor() override;
    void subscribeOnStateChange(OnStateChange onStateChange);

    private:
    BinaryState m_state{BinaryState::Low};
    BinaryState m_lastState{BinaryState::Low};
    unsigned long m_lastPress{};
    static constexpr unsigned long DEBOUNCE_DELAY {50}; // ms
    OnStateChange m_onStateChangeCb{};

};

#endif