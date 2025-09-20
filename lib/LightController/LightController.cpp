#include "LightController.h"
#include "Config.h"

LightController::LightController(Light& light, Button& onOffButton, PirSensor& pirSensor, MqttHandler* mqttHandler) :
    m_light{light},    
    m_powerButton{onOffButton},
    m_pirSensor{pirSensor},
    m_mqttHandler{mqttHandler}
{}

void LightController::init()
{
    m_powerButton.subscribeOnStateChange([this](const Button::BinaryState& state){
        if(state == Button::BinaryState::High)
            m_light.toggle();
    });

    m_pirSensor.subscribeOnStateChange([this](const PirSensor::BinaryState& state){
        if(state == PirSensor::BinaryState::High && m_light.getState() == Light::State::Off)
            m_light.turnOn();

        if(state == PirSensor::BinaryState::Low && m_light.getState() == Light::State::On)
        {
            m_light.turnOff();
        }
    });

    m_light.subscribeOnStateChange([this](const Light::State& state){
        if(!m_mqttHandler)
        {
            Serial.println("MQTT Handler is null (skipping publishing state)");
            return;
        }
        m_mqttHandler->setPublishingTopics(std::string{Config::Mqtt::Topics::STATE}, m_light.stateToStr(state));
    });

    m_light.subscribeOnBrightnessChange([this](int brightness){
        if(!m_mqttHandler)
        {
            Serial.println("MQTT Handler is null (skipping publishing brightness)");
            return;
        }
        m_mqttHandler->setPublishingTopics(std::string{Config::Mqtt::Topics::BRIGHTNESS}, std::to_string(brightness));
    });

    if(m_mqttHandler)
    {
        m_mqttHandler->subscribeTopic(std::string{Config::Mqtt::Topics::STATE_SET}, [this](const std::string& payload) {
            std::string upperPayload{payload};
            std::transform(upperPayload.begin(), upperPayload.end(), upperPayload.begin(), [](unsigned char c){return std::toupper(c);});
            if(upperPayload == "ON") m_light.setState(Light::State::On); 
            if(upperPayload == "OFF") m_light.setState(Light::State::Off); 
        });
        
        m_mqttHandler->subscribeTopic(std::string{Config::Mqtt::Topics::BRIGHTNESS_SET}, [this](const std::string& payload) { 
            try
            {
                int brightness = std::stoi(payload);
                m_light.setBrightness(brightness);
            }
            catch(const std::exception& e)
            {
                Serial.println("Invalid brightness payload: " + String(payload.c_str()));
            }
        });

        // Set default values for publishing topics
        m_mqttHandler->setPublishingTopics(std::string{Config::Mqtt::Topics::STATE}, m_light.stateToStr(m_light.getState()));
        m_mqttHandler->setPublishingTopics(std::string{Config::Mqtt::Topics::BRIGHTNESS}, std::to_string(m_light.getBrightness()));
    }
}