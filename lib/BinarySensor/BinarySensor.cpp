#include "BinarySensor.h"

void BinarySensor::subscribeOnStateChange(OnStateChange onStateChange)
{
    m_onStateChangeCb = std::move(onStateChange);
}

void BinarySensor::notifyStateChange(BinaryState newState)
{
    if(newState != m_state)
    {
        m_state = newState;

        if(m_onStateChangeCb)
            m_onStateChangeCb(m_state);
    }
}