#ifndef LIGHTCONTROLLER_H
#define LIGHTCONTROLLER_H

#include "PirSensor.h"
#include "Button.h"
#include "BH1750.h"
#include "Light.h"
#include "MqttLightBridge.h"
#include <typeindex>

class LightController
{
    public:
    struct LightEntry
    {
        std::shared_ptr<Light> lightPtr{};
        std::vector<std::shared_ptr<Sensor>> sensors{};
    };

    LightController(MqttLightBridge* m_mqttLightBridge = nullptr);

    void setupDevices();
    void subscribe();
    void addLight(std::shared_ptr<Light> light, std::vector<std::shared_ptr<Sensor>> sensors = {});
    void update();

    private:
    std::unordered_map<std::string, LightEntry> m_lights{};
    std::unordered_map<std::string, std::string> m_discoveryPayloadCache{};
    MqttLightBridge* m_mqttLightBridge{};

    void handleBinarySensorStateChange(const std::string& lightId, const BinarySensor* binarySensor, BinarySensor::BinaryState state);

    template<typename T>
    std::vector<T*> getSensors(const std::string& lightId);
};

template <typename T>
std::vector<T*> LightController::getSensors(const std::string& lightId)
{
    std::vector<T*> sensorPtrs{};
    auto lightIt{m_lights.find(lightId)};

    if(lightIt == m_lights.end())
    {
        Serial.printf("[LightController] LightId %s does not exists!\n", lightId.c_str());
        return sensorPtrs;
    }

    auto& sensors{lightIt->second.sensors};

    for(auto& sensor : sensors)
    {
        auto sensorPtr{dynamic_cast<T*>(sensor.get())};
        if(sensorPtr)
            sensorPtrs.emplace_back(sensorPtr);
    }

    return sensorPtrs;
}

#endif