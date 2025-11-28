#include "PirSensor.h"

PirSensor::PirSensor(int pin) : BinarySensor(pin) 
{}

void PirSensor::init()
{
    pinMode(m_pin, INPUT);
}

void PirSensor::update()
{
    BinaryState state = static_cast<BinaryState>(digitalRead(m_pin));
    BinarySensor::notifyStateChange(state);
}