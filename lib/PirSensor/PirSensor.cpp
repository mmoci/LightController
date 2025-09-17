#include "PirSensor.h"

PirSensor::PirSensor(int pin) : BinarySensor(pin) 
{}

void PirSensor::init()
{
    pinMode(m_pin, INPUT);
}

void PirSensor::subscribeOnStateChange(OnStateChange onStateChange)
{
    m_onStateChangeCb = std::move(onStateChange);
}

PirSensor::BinaryState PirSensor::readSensor()
{
    BinaryState state = static_cast<BinaryState>(digitalRead(m_pin));

    if(state != m_state)
    {
        m_state = state;
        
        if (m_onStateChangeCb)
            m_onStateChangeCb(m_state);
    }

    return m_state;
}