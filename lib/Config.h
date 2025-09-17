#ifndef CONFIG_H
#define CONFIG_H

#include <string_view>
#include <Arduino.h>

namespace Config
{
    // WiFi configuration variables
    namespace WiFi
    {
        constexpr std::string_view WIFI_SSID = "wifi";
        constexpr std::string_view WIFI_PASSWORD = "pass";
        const IPAddress STATIC_IP(192,168,1,16);
        const IPAddress GATEWAY(192,168,1,1);
        const IPAddress SUBNET(255,255,255,0);
        const IPAddress DNS1(85,94,64,10);
        const IPAddress DNS2(85,94,64,11);
    }

    // MQTT configuration variables
    namespace Mqtt
    {
        constexpr int RETRIES = 100;
        constexpr std::string_view SERVER = "192.168.1.2";
        constexpr int              PORT = 1883;
        constexpr std::string_view USERNAME = "mqtt";
        constexpr std::string_view PASSWORD = "mqtt";
        constexpr std::string_view CLIENT_NAME = "lightA";
        

        /*
        For now (hardcoded topics in Config.h):
        Use std::string_view for keys in m_onMessageReceivedCbMap and function parameter → avoids unnecessary copies.
        Later (multiple lights, dynamic topic generation):
        Either:
        Keep everything as std::string, or
        Store owned std::string in a container and use std::string_view as lookup key (possible but trickier).
        */
        namespace Topics 
        {
            constexpr std::string_view DISCOVERY      = "homeassistant/light/lightA/config";
            constexpr std::string_view STATE          = "homeassistant/light/lightA/state";
            constexpr std::string_view STATE_SET      = "homeassistant/light/lightA/state/set";
            constexpr std::string_view BRIGHTNESS     = "homeassistant/light/lightA/brightness";
            constexpr std::string_view BRIGHTNESS_SET = "homeassistant/light/lightA/brightness/set";
            constexpr std::string_view AVAILABILITY   = "homeassistant/light/lightA/availability";
        }

        // Hardcoded discovery payload
        constexpr std::string_view DISCOVERY_PAYLOAD = R"({
            "name": "Light A",
            "unique_id": "lightA",
            "command_topic": "homeassistant/light/lightA/state/set",
            "state_topic": "homeassistant/light/lightA/state",
            "brightness_command_topic": "homeassistant/light/lightA/brightness/set",
            "brightness_state_topic": "homeassistant/light/lightA/brightness",
            "availability_topic": "homeassistant/light/lightA/availability",
            "payload_on": "ON",
            "payload_off": "OFF",
            "optimistic": false,
            "brightness": true
        })";
    }
}

#endif