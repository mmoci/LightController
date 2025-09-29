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
        const IPAddress DNS1(8,8,8,8);
        const IPAddress DNS2(8,8,4,4);
    }

    // MQTT configuration variables
    namespace Mqtt
    {
        constexpr int RETRIES = 100;
        constexpr std::string_view SERVER = "192.168.1.2";
        constexpr int              PORT = 1883;
        constexpr std::string_view USERNAME = "mqtt";
        constexpr std::string_view PASSWORD = "mqtt";
        constexpr std::string_view CLIENT_NAME = "LightController_1";
    }
}

#endif