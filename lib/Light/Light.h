#ifndef LIGHT_H
#define LIGHT_H

#include <algorithm>
#include <functional>

class Light
{
    public:
    enum class State
    {
        Off,
        On
    };

    using OnStateChange = std::function<void(const State&)>;
    using OnBrightnessChange = std::function<void(int)>;

    // Default ledChannel = 0, PWM frequency 10kHz, 8-bit resolution
    Light(int pin, int ledChannel = 0, int frequency = 10000, int resolution = 8);

    void init();
    void setState(const State& state);
    void setBrightness(int brightness);
    State getState() const;
    int getBrightness() const;
    void turnOn();
    void turnOff();
    void toggle();
    void subscribeOnStateChange(OnStateChange onStateChange);
    void subscribeOnBrightnessChange(OnBrightnessChange onStateChange);
    std::string stateToStr(const Light::State& state);

    private:
    int m_pwmPin{};
    int m_ledChannel{};
    int m_frequency{};
    int m_resolution{};
    int m_brightness{};
    State m_state{};
    OnStateChange m_onStateChangeCb{};
    OnBrightnessChange m_onBrightnessChangeCb{};
};

#endif