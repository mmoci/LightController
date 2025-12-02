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
    for(const auto& [lightId, lightEntry] : m_lights)
    {
        for(const auto& sensorPtr : lightEntry.sensors)
            sensorPtr->init();

        lightEntry.lightPtr->init();
    }
}

void LightController::update()
{
    for(const auto& [lightId, lightEntry] : m_lights)
    {
        for(const auto& sensorPtr : lightEntry.sensors)
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
        auto lightPtrWeak = std::weak_ptr<Light>(lightPtr);

        // Subscribe to state change on all binary sensors
        for(const auto& sensorPtr : lightEntry.sensors)
        {
            auto binarySensor {std::dynamic_pointer_cast<BinarySensor>(sensorPtr)};
            auto binarySensorWeak {std::weak_ptr<BinarySensor>(binarySensor)};
            if(binarySensor)
            {
                binarySensor->subscribeOnStateChange([this, lightId, binarySensorWeak](BinarySensor::BinaryState state){
                    if(binarySensorWeak.expired())
                    {
                        Serial.printf("[LightController] LightId %s: binary sensor expired!\n", lightId.c_str());
                        return;
                    }
                    
                    auto binarySensor{binarySensorWeak.lock()};

                    handleBinarySensorStateChange(lightId, binarySensor, state);
                });
            }
        }

        if(m_mqttLightBridge)
        {
            // Subscribe to state change on the light
            lightPtr->subscribeOnStateChange([this, lightId, lightPtrWeak](const Light::State& state){
                if(lightPtrWeak.expired())
                {
                    Serial.printf("[LightController] LightId %s: state change subscribe is discarded!\n", lightId.c_str());
                    return;
                }

                auto lightPtr{lightPtrWeak.lock()};

                m_mqttLightBridge->publishStateChange(lightId, lightPtr->getState());
            });

            // Subscribe to brightness change on the light
            lightPtr->subscribeOnBrightnessChange([this, lightId, lightPtrWeak](int brightness){
                if(lightPtrWeak.expired())
                {
                    Serial.printf("[LightController] LightId %s: brightness subscribe is discarded!\n", lightId.c_str());
                    return;
                }

                auto lightPtr{lightPtrWeak.lock()};

                m_mqttLightBridge->publishBrightnessChange(lightId, lightPtr->getBrightness());
            });

            m_mqttLightBridge->registerCommandHandlers(lightId, [lightId, lightPtrWeak](const std::string& commandType, const std::string& payload){
                if(lightPtrWeak.expired())
                {
                    Serial.printf("[LightController] LightId %s: message is discarded!\n", lightId.c_str());
                    return;
                } 

                auto lightPtr{lightPtrWeak.lock()};
                    
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

void LightController::handleBinarySensorStateChange(const std::string& lightId, const std::shared_ptr<BinarySensor> binarySensor, BinarySensor::BinaryState state)
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
                for(const auto& luminanceSensor : luminanceSensors)
                {
                    if(!luminanceSensor.expired() && luminanceSensor.lock()->isLowLight())
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
