#ifndef LIGHTCONTROLLER_H
#define LIGHTCONTROLLER_H

#include <PirSensor.h>
#include <Button.h>
#include <Light.h>
#include <MqttHandler.h>

class LightController
{
    public:
    LightController(Light& light, Button& powerButton, PirSensor& pirSensor, MqttHandler* m_mqttHandler = nullptr);

    void init();

    private:
    Button& m_powerButton;
    PirSensor& m_pirSensor;
    Light& m_light;
    MqttHandler* m_mqttHandler{};
};

#endif