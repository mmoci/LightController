#ifndef MQTTLIGHTBRIDGE_H
#define MQTTLIGHTBRIDGE_H

#include "MqttHandler.h"
#include "Light.h"

class MqttLightBridge
{
    using OnLightCommandCb = std::function<void(const std::string&, const std::string&)>;

    private:
    std::unordered_map<std::string, std::shared_ptr<Light>> m_lights{};
    std::unordered_map<std::string, std::string> m_discoveryPayloadCache{};
    MqttHandler* m_mqttHandler{};

    std::string getDiscoveryPayload(const std::string& lightId) const;
    std::string createDiscoveryPayload(const std::string& lightId) const;

    public:
    MqttLightBridge(MqttHandler* handler = nullptr) : m_mqttHandler{handler}
    {}

    void publishDiscoveryTopics();
    void publishInitialStates(); 
    void addLight(std::shared_ptr<Light> lightPtr);
    void publishStateChange(const std::string& lightId, Light::State state);
    void publishBrightnessChange(const std::string& lightId, int brightness);
    void registerCommandHandlers(const std::string& lightId, OnLightCommandCb onLightCommandCb);
};

#endif