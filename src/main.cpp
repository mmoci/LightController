#include <Arduino.h>
#include <BinarySensor.h>
#include <PirSensor.h>
#include <Button.h>
#include <Light.h>
#include <LightController.h>

#ifdef ENABLE_MQTT
#include <MqttHandler.h>
#endif

constexpr int PIR_PIN                   {25};
constexpr int BUTTON_PIN                {33};
constexpr int LIGHT_PIN                 {32};
constexpr std::string_view LIGHT_1_NAME {"Podrum"};
constexpr std::string_view LIGHT_1_ID   {"podrum"};

#ifdef ENABLE_MQTT
MqttHandler mqttHandler{};
MqttLightBridge mqttLightBridge{&mqttHandler};
LightController controller{&mqttLightBridge};
#else
LightController controller{};
#endif

void setup() 
{
  Serial.begin(115200);

  std::shared_ptr<Light> ledLight{std::make_shared<Light>(LIGHT_1_NAME, LIGHT_1_ID, LIGHT_PIN)};
  std::vector<std::shared_ptr<BinarySensor>> binarySensors{};
  binarySensors.emplace_back(std::make_shared<Button>(BUTTON_PIN));
  binarySensors.emplace_back(std::make_shared<PirSensor>(PIR_PIN));
  controller.addLight(std::move(ledLight), std::move(binarySensors));
  controller.setupDevices();

  #ifdef ENABLE_MQTT
  mqttHandler.init(Config::Mqtt::SERVER, Config::Mqtt::PORT, Config::Mqtt::CLIENT_NAME);
  #endif
  
  controller.subscribe();

  #ifdef ENABLE_MQTT
  while(!mqttHandler.connect())
  {
    delay(5000);
  }
  #endif
}

void loop() 
{
  controller.update();
  #ifdef ENABLE_MQTT
  mqttHandler.process();
  #endif
}

