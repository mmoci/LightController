#include "PirSensor.h"

PirSensor::PirSensor(int pin) : BinarySensor(pin) 
{}

void PirSensor::init()
{
    pinMode(m_pin, INPUT);
}

PirSensor::BinaryState PirSensor::readSensor()
{
    BinaryState state = static_cast<BinaryState>(digitalRead(m_pin));

    BinarySensor::notifyStateChange(state);

    return BinarySensor::getState();
}

void PirSensor::bindToLight(std::shared_ptr<Light> lightPtr)
{
    subscribeOnStateChange([lightWeakPtr = std::weak_ptr<Light>(lightPtr)](PirSensor::BinaryState state){
        auto lightPtr = lightWeakPtr.lock();
        if(state == PirSensor::BinaryState::High && lightPtr && lightPtr->getState() == Light::State::Off)
            lightPtr->turnOn();

        if(state == PirSensor::BinaryState::Low && lightPtr && lightPtr->getState() == Light::State::On)
            lightPtr->turnOff();
    });
}