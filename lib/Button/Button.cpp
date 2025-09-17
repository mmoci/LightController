#include "Button.h"

Button::Button(int pin) : BinarySensor(pin)
{}

void Button::init()
{
    pinMode(m_pin, INPUT);
}

void Button::subscribeOnStateChange(OnStateChange onStateChange)
{
    m_onStateChangeCb = std::move(onStateChange);
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
        if(state != m_state)
        {
            m_state = state;

            if(m_onStateChangeCb)
                m_onStateChangeCb(m_state);
        }
    }

    m_lastState = state;
    return m_state;
}