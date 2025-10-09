#include "LightController.h"
#include "Config.h"
#include "MqttTopicBuilder.h"

LightController::LightController(MqttLightBridge* mqttLightBridge) : m_mqttLightBridge{mqttLightBridge}
{}

void LightController::addLight(std::shared_ptr<Light> light, std::vector<std::shared_ptr<BinarySensor>> binarySensors)
{
    m_lights.emplace(light->getId(), LightEntry{light, binarySensors});

    if(m_mqttLightBridge)
        m_mqttLightBridge->addLight(light);
}

void LightController::setupDevices()
{
    for(auto& [lightId, lightEntry] : m_lights)
    {
        for(auto& sensor : lightEntry.binarySensors)
            sensor->init();

        lightEntry.lightPtr->init();
    }
}

void LightController::update()
{
    for(auto& [lightId, lightEntry] : m_lights)
    {
        for(auto& sensor : lightEntry.binarySensors)
            sensor->readSensor();
    }
}

void LightController::subscribe()
{
    if(m_mqttLightBridge)
    {
        m_mqttLightBridge->publishDiscoveryTopics();
        m_mqttLightBridge->publishInitialStates();
    }

    // Subscribe to state change on all lights
    for(auto& [lightId, lightEntry] : m_lights)
    {
        auto* lightPtr = lightEntry.lightPtr.get();

        // Subscribe to state change on all binary sensors
        for(auto& sensor : lightEntry.binarySensors)
        {
            sensor->bindToLight(lightEntry.lightPtr);
        }

        if(m_mqttLightBridge)
        {
            // Subscribe to state change on the light
            lightPtr->subscribeOnStateChange([this, lightId, lightPtr](const Light::State& state){
                m_mqttLightBridge->publishStateChange(lightId, lightPtr->getState());
            });

            // Subscribe to brightness change on the light
            lightPtr->subscribeOnBrightnessChange([this, lightId, lightPtr](int brightness){
                m_mqttLightBridge->publishBrightnessChange(lightId, lightPtr->getBrightness());
            });

            m_mqttLightBridge->registerCommandHandlers(lightId, [lightPtr](const std::string& commandType, const std::string& payload){
                if(commandType == "state")
                {
                    if(payload == "ON") lightPtr->setState(Light::State::On); 
                    if(payload == "OFF") lightPtr->setState(Light::State::Off);
                }
                else if(commandType == "brightness")
                {
                    int brightness = std::stoi(payload);
                    lightPtr->setBrightness(brightness);
                }
                else
                {
                    Serial.println("Invalid command type: " + String(commandType.c_str()));
                }
            });
        }
    }
}
