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

void PirSensor::bindToLight(Light& light)
{
    subscribeOnStateChange([&light](PirSensor::BinaryState state){
        if(state == PirSensor::BinaryState::High && light.getState() == Light::State::Off)
            light.turnOn();

        if(state == PirSensor::BinaryState::Low && light.getState() == Light::State::On)
            light.turnOff();
    });
}