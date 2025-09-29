#ifndef LIGHTCONTROLLER_H
#define LIGHTCONTROLLER_H

#include "PirSensor.h"
#include "Button.h"
#include "Light.h"
#include "MqttHandler.h"

class LightController
{
    public:
    struct LightEntry
    {
        std::unique_ptr<Light> lightPtr{};
        std::vector<std::unique_ptr<BinarySensor>> binarySensors{};
    };

    LightController(MqttHandler* m_mqttHandler = nullptr);

    void setupDevices();
    void subscribe();
    void addLight(std::unique_ptr<Light> light, std::vector<std::unique_ptr<BinarySensor>> binarySensors = {});
    void update();

    private:
    std::string createDiscoveryPayload(const std::string& lightId) const;
    std::string getDiscoveryPayload(const std::string& lightId) const;

    std::unordered_map<std::string, LightEntry> m_lights{};
    std::unordered_map<std::string, std::string> m_discoveryPayloadCache{};
    MqttHandler* m_mqttHandler{};
};

#endif