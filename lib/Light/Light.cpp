#include "Light.h"
#include <Arduino.h>

Light::Light(std::string_view name, std::string_view id, int pin, int ledChannel, int frequency, int resolution) :
    m_name{name},
    m_id{id},
    m_pwmPin{pin},
    m_ledChannel{ledChannel},
    m_frequency{frequency},
    m_resolution{resolution},
    m_brightness{(1 << m_resolution) - 1},
    m_state{State::Off}
{}

void Light::init()
{
    // Configure LED PWM functionalitites
    ledcSetup(m_ledChannel, m_frequency, m_resolution);

    // Attach the channel to the GPIO to be controlled
    ledcAttachPin(m_pwmPin, m_ledChannel);

    #ifdef ESP32
    ledcWrite(m_ledChannel, 0); // For safty reason
    #elif defined(ESP8266)
    analogWrite(m_pwmPin, 0); // For safty reason
    #endif
}

std::string_view Light::getName() const
{
    return m_name;
}

std::string_view Light::getId() const
{
    return m_id;
}

void Light::setState(const State& state)
{
    if (state == State::On)
        turnOn();
    else
        turnOff();
}

Light::State Light::getState() const
{
    return m_state;
}

void Light::subscribeOnStateChange(OnStateChange onStateChange)
{
    m_onStateChangeCb = std::move(onStateChange);
}

void Light::setBrightness(int brightness)
{
    m_brightness = std::clamp(brightness, 0, (1 << m_resolution) - 1);

    if(m_brightness == 0)
        turnOff();
    else
        turnOn();

    if(m_onBrightnessChangeCb)
        m_onBrightnessChangeCb(m_brightness);
}

void Light::subscribeOnBrightnessChange(OnBrightnessChange onStateChange)
{
    m_onBrightnessChangeCb = std::move(onStateChange);
}

int Light::getBrightness() const
{
    return m_brightness;
}

void Light::turnOn()
{
    m_state = State::On;

    #ifdef ESP32
    // ledcWrite, input represents channel on ESP32
    ledcWrite(m_ledChannel, m_brightness);
    #else
    // analogWrite, input represents ESP8266 pin
    analogWrite(m_pwmPin, m_brightness);
    #endif

    if (m_onStateChangeCb) 
        m_onStateChangeCb(m_state);
}

void Light::turnOff()
{
    m_state = State::Off;

    #ifdef ESP32
    // ledcWrite, input represents channel on ESP32
    ledcWrite(m_ledChannel, 0);
    #else
    // analogWrite, input represents ESP8266 pin
    analogWrite(m_pwmPin, 0);
    #endif

    if (m_onStateChangeCb)
        m_onStateChangeCb(m_state);
}

void Light::toggle()
{
    setState(m_state == State::Off ? State::On : State::Off);
    Serial.println("Light is " + String(stateToStr(m_state).c_str()) + " brightness " + m_brightness);
}

std::string Light::stateToStr(Light::State state)
{
    switch(state)
    {
        case State::Off:
        return "OFF";

        case State::On:
        return "ON";

        default:
        return "Unknown";
    }
}