#ifndef LIGHTCONTROLLER_H
#define LIGHTCONTROLLER_H

#include "PirSensor.h"
#include "Button.h"
#include "BH1750.h"
#include "Light.h"
#include "MqttLightBridge.h"
#include <unordered_map>
#include <string>
#include <memory>

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
    MqttLightBridge* m_mqttLightBridge{};

    void handleBinarySensorStateChange(const std::string& lightId, const std::shared_ptr<BinarySensor> binarySensor, BinarySensor::BinaryState state);

    template<typename T>
    std::vector<std::weak_ptr<T>> getSensors(const std::string& lightId);
};

template <typename T>
std::vector<std::weak_ptr<T>> LightController::getSensors(const std::string& lightId)
{
    std::vector<std::weak_ptr<T>> sensorPtrs{};
    auto lightIt{m_lights.find(lightId)};

    if(lightIt == m_lights.end())
    {
        Serial.printf("[LightController] LightId %s does not exists!\n", lightId.c_str());
        return sensorPtrs;
    }

    auto& sensors{lightIt->second.sensors};

    for(auto& sensor : sensors)
    {
        auto sensorPtr{std::dynamic_pointer_cast<T>(sensor)};
        if(sensorPtr)
            sensorPtrs.emplace_back(sensorPtr);
    }

    return sensorPtrs;
}

#endif