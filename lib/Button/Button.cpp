#include "Button.h"

Button::Button(int pin) : BinarySensor(pin)
{}

void Button::init()
{
    pinMode(m_pin, INPUT);
}

Button::BinaryState Button::readSensor()
{
    BinaryState state = static_cast<BinaryState>(digitalRead(m_pin));

    if(state != m_lastState)
    {
        m_lastPress = millis();
    }

    if(millis() - m_lastPress > DEBOUNCE_DELAY)
    {
        BinarySensor::notifyStateChange(state);
    }

    m_lastState = state;
    return BinarySensor::getState();
}

void Button::bindToLight(Light& light)
{
    subscribeOnStateChange([&light](Button::BinaryState state){
        if(state == Button::BinaryState::High)
            light.toggle();
    });
}