#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <PubSubClient.h>
#ifdef ESP32
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Config.h>
#include <string_view>
#include <set>

class MqttHandler
{
    using OnMessageReceivedCb = std::function<void(const std::string&)>;

    public:
    enum class Availability
    {
        Offline,
        Online
    };

    MqttHandler() : m_mqttClient{m_wifiClient}
    {}

    void init(std::string_view mqttBroker, int port, std::string_view clientId);
    void setupWifi();
    bool connect();
    void process(); 
    void setPublishingTopics(const std::string& topic, const std::string& payload);
    void receiveMessages(char* topic, byte* payload, unsigned int length);
    void subscribeTopic(const std::string& topic,  OnMessageReceivedCb onMessageReceivedCb);
    Availability getAvailability() const;

    private:
    WiFiClient m_wifiClient{};
    PubSubClient m_mqttClient{};
    Availability m_availabilityState{Availability::Offline};
    std::string_view m_clientName{Config::Mqtt::CLIENT_NAME};
    std::string_view m_username{Config::Mqtt::USERNAME};
    std::string_view m_password{Config::Mqtt::PASSWORD};
    std::set<std::string> m_subscribingTopics{};
    std::unordered_map<std::string, std::pair<std::string, bool>> m_publishingTopics{};
    std::unordered_map<std::string, OnMessageReceivedCb> m_onMessageReceivedCbMap{};

    std::string availabilityToStr(const Availability& availablitiy);
    void getBaseTopic(std::string& topic);
    void publishTopic(std::string_view stringTopic, std::string_view stringPayload, boolean retain);
    void publishAllTopics();
    void subscribeTopic(std::string_view stringTopic);
    void subscribeAllTopics();
};

#endif