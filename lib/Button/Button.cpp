#include "Button.h"

Button::Button(int pin) : BinarySensor(pin)
{}

void Button::init()
{
    pinMode(m_pin, INPUT);
}

Button::BinaryState Button::readSensor()
{
    BinaryState currentState = static_cast<BinaryState>(digitalRead(m_pin));

    if (currentState != m_lastState) {
        m_lastChangeTime = millis();     // reset debounce timer
        m_lastState = currentState;      // update raw
    }

    if ((millis() - m_lastChangeTime) > DEBOUNCE_DELAY && currentState != m_stableState) {
        m_stableState = currentState;    // accept as stable
        BinarySensor::notifyStateChange(currentState);
    }

    return m_stableState;
}

void Button::bindToLight(std::shared_ptr<Light> lightPtr)
{
    subscribeOnStateChange([lightWeakPtr = std::weak_ptr<Light>(lightPtr)](Button::BinaryState state){
        auto lightPtr = lightWeakPtr.lock();
        if(lightPtr && state == Button::BinaryState::High)
            lightPtr->toggle();
    });
}