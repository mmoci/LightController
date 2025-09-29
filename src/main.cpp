#include <Arduino.h>
#include <BinarySensor.h>
#include <PirSensor.h>
#include <Button.h>
#include <Light.h>
#include <LightController.h>
#include <MqttHandler.h>


constexpr int PIR_PIN = 1;
constexpr int BUTTON_PIN = 2;
constexpr int LIGHT_PIN = 3;

// Create a sensor objects.
MqttHandler mqttHandler{};
LightController controller{&mqttHandler};

void setup() 
{
  Serial.begin(115200);

  std::unique_ptr<Light> ledLight{std::make_unique<Light>("Kitchen Light", "kitchenLight", LIGHT_PIN)};
  std::vector<std::unique_ptr<BinarySensor>> binarySensors{};
  binarySensors.emplace_back(std::make_unique<Button>(BUTTON_PIN));
  binarySensors.emplace_back(std::make_unique<PirSensor>(PIR_PIN));
  controller.addLight(std::move(ledLight), std::move(binarySensors));
  controller.setupDevices();
  mqttHandler.init(Config::Mqtt::SERVER, Config::Mqtt::PORT, Config::Mqtt::CLIENT_NAME);
  controller.subscribe();

  while(!mqttHandler.connect())
  {
    delay(5000);
  }
}

void loop() 
{
  controller.update();
  mqttHandler.process();
}

