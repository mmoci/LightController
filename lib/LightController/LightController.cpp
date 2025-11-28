#include "LightController.h"
#include "Config.h"
#include "MqttTopicBuilder.h"

LightController::LightController(MqttLightBridge* mqttLightBridge) : m_mqttLightBridge{mqttLightBridge}
{}

void LightController::addLight(std::shared_ptr<Light> light, std::vector<std::shared_ptr<Sensor>> sensors)
{
    m_lights.emplace(light->getId(), LightEntry{light, sensors});

    if(m_mqttLightBridge)
        m_mqttLightBridge->addLight(light);
}

void LightController::setupDevices()
{
    for(auto& [lightId, lightEntry] : m_lights)
    {
        for(auto& sensorPtr : lightEntry.sensors)
            sensorPtr->init();

        lightEntry.lightPtr->init();
    }
}

void LightController::update()
{
    for(auto& [lightId, lightEntry] : m_lights)
    {
        for(auto& sensorPtr : lightEntry.sensors)
            sensorPtr->update();
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
        auto lightPtr{lightEntry.lightPtr};

        // Subscribe to state change on all binary sensors
        for(auto& sensorPtr : lightEntry.sensors)
        {
            auto binarySensor {std::dynamic_pointer_cast<BinarySensor>(sensorPtr)};
            if(binarySensor)
            {
                binarySensor->subscribeOnStateChange([this, lightId, binarySensor](BinarySensor::BinaryState state){
                    handleBinarySensorStateChange(lightId, binarySensor.get(), state);
                });
            }
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

void LightController::handleBinarySensorStateChange(const std::string& lightId, const BinarySensor* binarySensor, BinarySensor::BinaryState state)
{
    auto lightIt {m_lights.find(lightId)};

    if(lightIt == m_lights.end())
    {
        Serial.printf("[LightController] LightId %s does not exists!\n", lightId.c_str());
        return;
    }

    auto lightPtr{lightIt->second.lightPtr};
    auto sensorType{binarySensor->getSpecificType()};

    switch(sensorType)
    {
        case Sensor::SpecificType::Button:
            if(lightPtr && state == Button::BinaryState::High)
                lightPtr->toggle();
            break;

        case Sensor::SpecificType::PirSensor:
            if(state == BinarySensor::BinaryState::High && lightPtr && lightPtr->getState() == Light::State::Off)
            {
                auto luminanceSensors {getSensors<BH1750>(lightId)};
                for(auto& luminanceSensor : luminanceSensors)
                {
                    if(luminanceSensor->isLowLight())
                        lightPtr->turnOn();
                }
            }

            if(state == BinarySensor::BinaryState::Low && lightPtr && lightPtr->getState() == Light::State::On)
            {
                lightPtr->turnOff();
            }
            break;
        
        default:
        Serial.printf("[LightController] Not supported sensor type %d\n", sensorType);
    }
}
