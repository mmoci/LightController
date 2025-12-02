#ifndef BINARYSENSOR_H
#define BINARYSENSOR_H

#include "Sensor.h"
#include "Light.h"
#include <functional>

class BinarySensor : public Sensor
{
    public:
    enum class BinaryState
    {
        Low,
        High
    };

    using OnStateChange = std::function<void(BinaryState)>;

    BinarySensor(int pin) : m_pin{pin} {}
    virtual ~BinarySensor() = default;

    void subscribeOnStateChange(OnStateChange onStateChange);
    BinaryState getState() const noexcept {return m_state;}

    protected:
    int m_pin{};

    void notifyStateChange(BinaryState newState);

    private:
    BinaryState m_state{BinaryState::Low};
    OnStateChange m_onStateChangeCb{};
};

#endif