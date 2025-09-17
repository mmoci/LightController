#include <Arduino.h>
#include <PirSensor.h>
#include <Button.h>
#include <Light.h>
#include <LightController.h>
#include <MqttHandler.h>


constexpr int PIR_PIN = 1;
constexpr int BUTTON_PIN = 2;
constexpr int LIGHT_PIN = 3;

// Create a PirSensor object.
PirSensor pirSensor{PIR_PIN};
Button powerButton{BUTTON_PIN};
Light ledLight{LIGHT_PIN};
MqttHandler mqttHandler{};
LightController controller{ledLight, powerButton, pirSensor, &mqttHandler};

void setup() 
{
  Serial.begin(115200);

  powerButton.init();
  pirSensor.init();
  ledLight.init();
  mqttHandler.init(Config::Mqtt::SERVER, Config::Mqtt::PORT, Config::Mqtt::CLIENT_NAME);
  controller.init();

  while(!mqttHandler.connect())
  {
    delay(5000);
  }
}

void loop() 
{
  powerButton.readSensor();
  pirSensor.readSensor();
  mqttHandler.process();
}
