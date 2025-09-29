#include "MqttHandler.h"
#include "Config.h"
#include "MqttTopicBuilder.h"

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
  WiFi.config(Config::WiFi::STATIC_IP, Config::WiFi::GATEWAY, Config::WiFi::SUBNET, Config::WiFi::DNS1, Config::WiFi::DNS2); //For statis IP address
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
    const std::string availabilityTopic{MqttTopicBuilder::getAvailabilityTopic()};
    //const std::string discoveryTopic{MqttTopicBuilder::getDiscoveryTopic()};
    //const std::string discoveryPayload{createDiscoveryPayload(lightId, lightName)};

    const std::string willTopic{availabilityTopic};
    const std::string willPayload{availabilityToStr(Availability::Offline)};

    if (m_mqttClient.connect(m_clientName.data(), 
                             m_username.data(), 
                             m_password.data(),
                             willTopic.c_str(),
                             willQoS, willRetain, willPayload.c_str())) 
    {
        Serial.println("Connected to MQTT broker");
        
        m_availabilityState = Availability::Online;
        if(m_onMqttConnectCb) m_onMqttConnectCb();
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

void MqttHandler::enqueuePublishTopic(const std::string& topic, const std::string& payload)
{
    auto it = m_publishingTopics.find(topic);

    if(it != m_publishingTopics.end())
    {
        it->second = {payload, true};
    } 
    else 
        Serial.println("Ignoring unknown publishing topic: " + String(topic.data()));
}

void MqttHandler::publishNowTopic(std::string_view stringTopic, std::string_view stringPayload, boolean retain)
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
    for(auto& [topic, payloadData] : m_publishingTopics)
    {
        auto& [payload, shouldPublish] = payloadData;

        if(shouldPublish && !payload.empty())
        {
            publishNowTopic(topic, payload, true);
            shouldPublish = false;
        }
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

void MqttHandler::subscribeTopic(const std::string& topic,  OnMessageReceivedCb onMessageReceivedCb)
{
    m_subscribingTopics.insert(topic);
    subscribeTopic(topic);
    m_onMessageReceivedCbMap[topic] = std::move(onMessageReceivedCb);
}

void MqttHandler::receiveMessages(char* topic, byte* payload, unsigned int length)
{
    std::string strTopic{topic};
    std::string strMessage{reinterpret_cast<char*>(payload), length};

    Serial.println("Message arrived [" + String(strTopic.c_str()) + "] payload: " + String(strMessage.c_str()));

    auto it = m_onMessageReceivedCbMap.find(strTopic);
    if(it != m_onMessageReceivedCbMap.end())
    {
        it->second(strMessage);
    }
    else
        Serial.println("No match found for [" + String(strTopic.c_str()) + "]");
}

void MqttHandler::onMqttConnect(std::function<void()> onMqttConnectCb)
{
    m_onMqttConnectCb = std::move(onMqttConnectCb);
}

// This member function is currently not used - subscribing topics are stored in the map as key with /set postfix, no need to strip it.
void MqttHandler::getBaseTopic(std::string& topic)
{
    if(topic.size() > 4 && topic.compare(topic.size() - 4, 4, "/set") == 0)
    {
        topic.erase(topic.size() - 4);
    }
}

std::string MqttHandler::availabilityToStr(const Availability& availablitiy)
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