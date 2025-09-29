#include "LightController.h"
#include "Config.h"
#include "MqttTopicBuilder.h"

LightController::LightController(MqttHandler* mqttHandler) : m_mqttHandler{mqttHandler}
{}

void LightController::addLight(std::unique_ptr<Light> light, std::vector<std::unique_ptr<BinarySensor>> binarySensors)
{
    m_lights.emplace(light->getId(), LightEntry{std::move(light), std::move(binarySensors)});
    m_discoveryPayloadCache.emplace(light->getId(), createDiscoveryPayload(std::string{light->getId()}));
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
    if(m_mqttHandler)
    {
        m_mqttHandler->onMqttConnect([this](){

            auto availabilityPayload {m_mqttHandler->availabilityToStr(m_mqttHandler->getAvailability())};
            m_mqttHandler->publishNowTopic(MqttTopicBuilder::getAvailabilityTopic(), availabilityPayload, true);

            for(const auto& [lightId, lightEntry] : m_lights)
            {
                auto discoveryPayload{getDiscoveryPayload(lightId)};

                if(!discoveryPayload.empty())
                    m_mqttHandler->publishNowTopic(MqttTopicBuilder::getDiscoveryTopic(lightId), discoveryPayload, true);
            }
        });
    }

    // Subscribe to state change on all lights
    for(auto& [lightId, lightEntry] : m_lights)
    {
        auto* lightPtr = lightEntry.lightPtr.get();

        // Subscribe to state change on all binary sensors
        for(auto& sensor : lightEntry.binarySensors)
        {
            sensor->bindToLight(*lightPtr);
        }

        // Subscribe to state change on the light
        lightEntry.lightPtr->subscribeOnStateChange([this, lightId, lightPtr](const Light::State& state){
            if(!m_mqttHandler)
            {
                Serial.println("MQTT Handler is null (skipping publishing state)");
                return;
            }
            m_mqttHandler->enqueuePublishTopic(MqttTopicBuilder::getStateTopic(lightId), lightPtr->stateToStr(state));
        });

        // Subscribe to brightness change on the light
        lightEntry.lightPtr->subscribeOnBrightnessChange([this, lightId](int brightness){
            if(!m_mqttHandler)
            {
                Serial.println("MQTT Handler is null (skipping publishing brightness)");
                return;
            }
            m_mqttHandler->enqueuePublishTopic(MqttTopicBuilder::getBrightnessTopic(lightId), std::to_string(brightness));
        });
    
        if(m_mqttHandler)
        {
            m_mqttHandler->subscribeTopic(MqttTopicBuilder::getStateCommandTopic(lightId), [this, lightPtr](const std::string& payload) {
                std::string upperPayload{payload};
                std::transform(upperPayload.begin(), upperPayload.end(), upperPayload.begin(), [](unsigned char c){return std::toupper(c);});
                if(upperPayload == "ON") lightPtr->setState(Light::State::On); 
                if(upperPayload == "OFF") lightPtr->setState(Light::State::Off); 
            });
            
            m_mqttHandler->subscribeTopic(MqttTopicBuilder::getBrightnessCommandTopic(lightId), [this, lightPtr](const std::string& payload) { 
                try
                {
                    int brightness = std::stoi(payload);
                    lightPtr->setBrightness(brightness);
                }
                catch(const std::exception& e)
                {
                    Serial.println("Invalid brightness payload: " + String(payload.c_str()));
                }
            });

            // Set default values for publishing topics
            m_mqttHandler->enqueuePublishTopic(MqttTopicBuilder::getStateTopic(lightId), lightPtr->stateToStr(lightPtr->getState()));
            m_mqttHandler->enqueuePublishTopic(MqttTopicBuilder::getBrightnessTopic(lightId), std::to_string(lightPtr->getBrightness()));
        }
    }
}

std::string LightController::createDiscoveryPayload(const std::string& lightId) const
{
    auto it = m_lights.find(lightId);

    if(it == m_lights.end())
        return {};

    std::string lightName {it->second.lightPtr->getName()};

    return
        "{"
            "\"name\": \""                     + lightName + "\"," +
            "\"unique_id\": \""                + lightId + "\"," +
            "\"command_topic\": \""            + MqttTopicBuilder::getStateCommandTopic(lightId) + "\"," +
            "\"state_topic\": \""              + MqttTopicBuilder::getStateTopic(lightId) + "\"," +
            "\"brightness_command_topic\": \"" + MqttTopicBuilder::getBrightnessCommandTopic(lightId) + "\"," +
            "\"brightness_state_topic\": \""   + MqttTopicBuilder::getBrightnessTopic(lightId) + "\"," +
            "\"availability_topic\": \""       + MqttTopicBuilder::getAvailabilityTopic() + "\"," +
            "\"payload_on\": \"ON\","
            "\"payload_off\": \"OFF\","
            "\"optimistic\": false,"           // TODO: remove hardcodilg latter
            "\"brightness\": true,"            // TODO: remove hardcodilg latter
            "\"device\": {"
                "\"identifiers\": [\"" + std::string{Config::Mqtt::CLIENT_NAME} +"\"],"
                "\"name\": \"" + std::string{"Kitchen Light Controller"} + "\","
                "\"manufacturer\": \"Marko Mocilac\""
            "}"
        "}";
}

std::string LightController::getDiscoveryPayload(const std::string& lightId) const
{
    auto it{m_discoveryPayloadCache.find(lightId)};

    if(it == m_discoveryPayloadCache.end())
        return {};

    return it->second;
}
