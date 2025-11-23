#ifndef BINARYSENSOR_H
#define BINARYSENSOR_H

#include "Sensor.h"
#include "Light.h"

class BinarySensor : public Sensor
{
    public:
    enum class BinaryState
    {
        Low,
        High
    };

    using OnStateChange = std::function<void(const BinaryState&)>;

    BinarySensor(int pin) : m_pin{pin} {}
    virtual ~BinarySensor() = default;

    virtual void init() = 0;
    virtual BinaryState readSensor() = 0;
    virtual void bindToLight(std::shared_ptr<Light> lightPtr) = 0;

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