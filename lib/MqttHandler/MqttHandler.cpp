#include "MqttHandler.h"
#include "Config.h"

void MqttHandler::init(std::string_view mqttBroker, int port, std::string_view clientId)
{
    setupWifi();
    m_mqttClient.setServer(mqttBroker.data(), port);
    m_mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        receiveMessages(topic, payload, length);
    });
}

void MqttHandler::setupWifi()
{  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(Config::WiFi::WIFI_SSID.data());

  // Set WIFI mode, WIFI_STA - Station (STA) mode is used to get ESP module connected to a WiFi network established by an access point
  WiFi.mode(WIFI_STA);
  WiFi.config(Config::WiFi::STATIC_IP, Config::WiFi::GATEWAY, Config::WiFi::SUBNET); //For statis IP address
  WiFi.begin(Config::WiFi::WIFI_SSID.data(), Config::WiFi::WIFI_PASSWORD.data());

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

bool MqttHandler::connect() {
    Serial.println("Attempting MQTT connection...");

    const int willQoS = 0;
    const bool willRetain = true;
    const std::string willTopic{Config::Mqtt::Topics::AVAILABILITY};
    const std::string willPayload{availablityToStr(Availability::Offline)};

    if (m_mqttClient.connect(m_clientName.data(), 
                             m_username.data(), 
                             m_password.data(),
                             willTopic.c_str(),
                             willQoS, willRetain, willPayload.c_str())) 
    {
        Serial.println("Connected to MQTT broker");
        m_availabilityState = Availability::Online;

        publishTopic(Config::Mqtt::Topics::AVAILABILITY, availablityToStr(m_availabilityState), true);
        publishTopic(Config::Mqtt::Topics::DISCOVERY, Config::Mqtt::DISCOVERY_PAYLOAD, true);
        subscribeAllTopics();
        return true;
    } 

    Serial.println("Failed to connect, state=" + String(m_mqttClient.state()));
    return false;
}

void MqttHandler::process() 
{
    static unsigned long lastReconnectAttempt = 0;

    if (!m_mqttClient.connected()) 
    {
        m_availabilityState = Availability::Offline;

        unsigned long now = millis();
        if (now - lastReconnectAttempt > 1000) 
        {
            lastReconnectAttempt = now;
            
            if (connect()) 
                lastReconnectAttempt = 0; // reset timer on success
        }
    } 
    else
    {
        m_mqttClient.loop(); // process incoming messages
        publishAllTopics();
    } 
}

void MqttHandler::setPublishingTopics(std::string_view topic, const std::string& payload)
{
    auto it = m_publishingTopics.find(topic);

    if(it != m_publishingTopics.end()) 
        it->second = payload;
    else 
        Serial.println("Ignoring unknown publishing topic: " + String(topic.data()));
}

void MqttHandler::publishTopic(std::string_view stringTopic, std::string_view stringPayload, boolean retain)
{
    if(m_availabilityState == Availability::Online)
    {
        m_mqttClient.publish(stringTopic.data(), stringPayload.data(), retain);
        Serial.println("Publishing topic [" + String(stringTopic.data()) + "] payload: " + String(stringPayload.data()));
    }
}

void MqttHandler::publishAllTopics()
{
    // Publish all topics from the map
    for(auto& [topic, payload] : m_publishingTopics)
    {
        if(!payload.empty())
            publishTopic(topic, payload, true);
    }
}

void MqttHandler::subscribeTopic(std::string_view stringTopic)
{
    if(m_availabilityState == Availability::Online)
    {
        m_mqttClient.subscribe(stringTopic.data());
        Serial.println("Subscribing on topic [" + String(stringTopic.data()) + "]");
    }
}

void MqttHandler::subscribeAllTopics()
{
    // Subscribe to all topics from the list
    for(const auto& topic : m_subscribingTopics)
    {
        subscribeTopic(topic);
    }
}

void MqttHandler::subscribeOnMessageReceived(std::string_view topic,  OnMessageReceivedCb onMessageReceivedCb)
{
    std::string subscribingTopic{std::string{topic} + "/set"};
    m_subscribingTopics.insert(subscribingTopic);
    subscribeTopic(subscribingTopic);
    m_onMessageReceivedCbMap[topic] = std::move(onMessageReceivedCb);
}

void MqttHandler::receiveMessages(char* topic, byte* payload, unsigned int length)
{
    std::string strTopic{topic};
    std::string strMessage{reinterpret_cast<char*>(payload), length};

    Serial.println("Message arrived [" + String(strTopic.c_str()) + "] payload: " + String(strMessage.c_str()));

    getBaseTopic(strTopic);

    auto it = m_onMessageReceivedCbMap.find(strTopic);
    if(it != m_onMessageReceivedCbMap.end())
    {
        it->second(strMessage);
    }
    else
        Serial.println("No match found for [" + String(strTopic.c_str()) + "]");
}

void MqttHandler::getBaseTopic(std::string& topic)
{
    if(topic.size() > 4 && topic.compare(topic.size() - 4, 4, "/set") == 0)
    {
        topic.erase(topic.size() - 4);
    }
}

std::string MqttHandler::availablityToStr(const Availability& availablitiy)
{
    switch(availablitiy)
    {
        case Availability::Offline:
        return "offline";

        case Availability::Online:
        return "online";

        default:
        return "unknown";
    }
}

MqttHandler::Availability MqttHandler::getAvailability() const
{
    return m_availabilityState;
}