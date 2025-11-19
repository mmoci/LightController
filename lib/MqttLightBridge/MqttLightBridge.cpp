#include "MqttLightBridge.h"
#include "MqttTopicBuilder.h"

// Anonymus namesepace with helper function (internal linkage), not part of the class (modern approach instead of static)
namespace
{
    std::string stripCommandTopic(const std::string& commandTopic)
    {
        auto endPos{commandTopic.find_last_of("/")};
        if(endPos == std::string::npos) return commandTopic;

        auto startPos{commandTopic.find_last_of("/", endPos - 1)};
        if(startPos == std::string::npos) return commandTopic.substr(0, endPos);

        return commandTopic.substr(startPos + 1, endPos - startPos - 1);
    }
}

void MqttLightBridge::addLight(std::shared_ptr<Light> lightPtr)
{
    if (m_lights.find(std::string{lightPtr->getId()}) != m_lights.end())
    {
        Serial.println("[Bridge] Light already registered, skipping.");
        return;
    }

    m_lights.emplace(lightPtr->getId(), lightPtr);
    m_discoveryPayloadCache.emplace(lightPtr->getId(), createDiscoveryPayload(std::string{lightPtr->getId()}));
}

void MqttLightBridge::publishDiscoveryTopics()
{
    if(!m_mqttHandler) 
    {
        Serial.println("[Bridge] Skipping publish discovery topics: MQTT handler not initialized");
        return;
    }

    m_mqttHandler->onMqttConnect([this](){

        auto availabilityPayload {m_mqttHandler->availabilityToStr(m_mqttHandler->getAvailability())};
        m_mqttHandler->publishNowTopic(MqttTopicBuilder::getAvailabilityTopic(), availabilityPayload, true);

        for(const auto& [lightId, light] : m_lights)
        {
            auto discoveryPayload{getDiscoveryPayload(lightId)};

            if(!discoveryPayload.empty())
                m_mqttHandler->publishNowTopic(MqttTopicBuilder::getDiscoveryTopic(lightId), discoveryPayload, true);
        }
    });
}

void MqttLightBridge::publishInitialStates() 
{
    if(!m_mqttHandler) 
    {
        Serial.println("[Bridge] Skipping publish initial states: MQTT handler not initialized");
        return;
    }

    for (const auto& [lightId, light] : m_lights) 
    {
        m_mqttHandler->enqueuePublishTopic(MqttTopicBuilder::getStateTopic(lightId), light->stateToStr(light->getState()));
        m_mqttHandler->enqueuePublishTopic(MqttTopicBuilder::getBrightnessTopic(lightId), std::to_string(light->getBrightness()));
    }
}

void MqttLightBridge::publishStateChange(const std::string& lightId, Light::State state)
{
    auto it = m_lights.find(lightId);

    if(it == m_lights.end())
    {
        Serial.println("[Bridge] Skipping publishing light state: unknown lightId");
        return;
    }    

    if(!m_mqttHandler)
    {
        Serial.println("[Bridge] Skipping publish: MQTT handler not initialized");
        return;
    }

    m_mqttHandler->enqueuePublishTopic(MqttTopicBuilder::getStateTopic(lightId), it->second->stateToStr(state));
}

void MqttLightBridge::publishBrightnessChange(const std::string& lightId, int brightness)
{
    auto it = m_lights.find(lightId);

    if(it == m_lights.end())
    {
        Serial.println("[Bridge] Skipping publishing brightness: unknown lightId");
        return;
    }    

    if(!m_mqttHandler)
    {
        Serial.println("[Bridge] Skipping publish: MQTT handler not initialized");
        return;
    }
    m_mqttHandler->enqueuePublishTopic(MqttTopicBuilder::getBrightnessTopic(lightId), std::to_string(brightness));
}

void MqttLightBridge::registerCommandHandlers(const std::string& lightId, OnLightCommandCb onLightCommandCb)
{
    auto it = m_lights.find(lightId);

    if(it == m_lights.end())
    {
        Serial.println("[Bridge] Skipping state publish: unknown lightId");
        return;
    }

    if(!m_mqttHandler)
    {
        Serial.println("[Bridge] Skipping publish: MQTT handler not initialized");
        return;
    }
    
    auto commandTopic{MqttTopicBuilder::getStateCommandTopic(lightId)};

    m_mqttHandler->subscribeTopic(commandTopic, [commandTopic, onLightCommandCb](const std::string& payload) {
        std::string upperPayload{payload};
        std::string strippedTopic{stripCommandTopic(commandTopic)};

        std::transform(upperPayload.begin(), upperPayload.end(), upperPayload.begin(), [](unsigned char c){return std::toupper(c);});
        if(upperPayload == "ON" || upperPayload == "OFF") {
            onLightCommandCb(strippedTopic, upperPayload);
        } 
        else 
        {
            Serial.println("Unsupported payload, do nothing");
        }
    });
    
    commandTopic = MqttTopicBuilder::getBrightnessCommandTopic(lightId);

    m_mqttHandler->subscribeTopic(commandTopic, [commandTopic, onLightCommandCb](const std::string& payload) { 
        try
        {
            onLightCommandCb(stripCommandTopic(commandTopic), payload);
        }
        catch(const std::exception& e)
        {
            Serial.println("[Bridge] Invalid brightness payload: " + String(payload.c_str()));
        }
    });
}

std::string MqttLightBridge::createDiscoveryPayload(const std::string& lightId) const
{
    auto it = m_lights.find(lightId);

    if(it == m_lights.end())
        return {};

    std::string lightName {it->second->getName()};

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
                "\"name\": \"" + std::string{"Light Controller"} + "\","
                "\"manufacturer\": \"Marko Mocilac\""
            "}"
        "}";
}

std::string MqttLightBridge::getDiscoveryPayload(const std::string& lightId) const
{
    auto it{m_discoveryPayloadCache.find(lightId)};

    if(it == m_discoveryPayloadCache.end())
        return {};

    return it->second;
}


