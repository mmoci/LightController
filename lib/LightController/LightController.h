#ifndef LIGHTCONTROLLER_H
#define LIGHTCONTROLLER_H

#include "PirSensor.h"
#include "Button.h"
#include "Light.h"
#include "MqttLightBridge.h"

class LightController
{
    public:
    struct LightEntry
    {
        std::shared_ptr<Light> lightPtr{};
        std::vector<std::shared_ptr<BinarySensor>> binarySensors{};
    };

    LightController(MqttLightBridge* m_mqttLightBridge = nullptr);

    void setupDevices();
    void subscribe();
    void addLight(std::shared_ptr<Light> light, std::vector<std::shared_ptr<BinarySensor>> binarySensors = {});
    void update();

    private:
    std::unordered_map<std::string, LightEntry> m_lights{};
    std::unordered_map<std::string, std::string> m_discoveryPayloadCache{};
    MqttLightBridge* m_mqttLightBridge{};
};

#endif