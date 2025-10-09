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

    BinarySensor(int pin) : Sensor(pin)
    {}

    virtual void init() = 0;
    virtual BinaryState readSensor() = 0;
    virtual void bindToLight(std::shared_ptr<Light> lightPtr) = 0;

    void subscribeOnStateChange(OnStateChange onStateChange);
    
    virtual ~BinarySensor() = default;

    BinaryState getState() const noexcept {return m_state;}

    protected:
    void notifyStateChange(BinaryState newState);

    private:
    BinaryState m_state{BinaryState::Low};
    OnStateChange m_onStateChangeCb{};
};

#endif