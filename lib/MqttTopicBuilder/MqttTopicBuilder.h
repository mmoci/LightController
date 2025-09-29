#ifndef MQTTTOPICBUILDER_H
#define MQTTTOPICBUILDER_H

#include <string>
#include "Config.h"

class MqttTopicBuilder
{
    public:
    static std::string getAvailabilityTopic();
    static std::string getDiscoveryTopic(const std::string& lightId);
    static std::string getStateTopic(const std::string& lightId);
    static std::string getStateCommandTopic(const std::string& lightId);
    static std::string getBrightnessTopic(const std::string& lightId);
    static std::string getBrightnessCommandTopic(const std::string& lightId);
};

inline std::string MqttTopicBuilder::getAvailabilityTopic()
{
    return "homeassistant/light/" + std::string{Config::Mqtt::CLIENT_NAME} + "/availability";
}

inline std::string MqttTopicBuilder::getDiscoveryTopic(const std::string& lightId)
{
    return "homeassistant/light/" + lightId + "/config";
}

inline std::string MqttTopicBuilder::getStateTopic(const std::string& lightId)
{
    return "homeassistant/light/" + lightId + "/state";
}

inline std::string MqttTopicBuilder::getStateCommandTopic(const std::string& lightId)
{
    return "homeassistant/light/" + lightId + "/state/set";
}

inline std::string MqttTopicBuilder::getBrightnessTopic(const std::string& lightId)
{
    return "homeassistant/light/" + lightId + "/brightness";
}

inline std::string MqttTopicBuilder::getBrightnessCommandTopic(const std::string& lightId)
{
    return "homeassistant/light/" + lightId + "/brightness/set";
}

#endif