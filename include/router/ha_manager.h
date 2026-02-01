#ifndef HAMANAGER_H
#define HAMANAGER_H

#include <Arduino.h>

#include "message_type.h"
#include "mqtt_client.h"

class HAManager {
   private:
    MqttClient& _mqttClient;

   public:
    HAManager(MqttClient& mqttClient) : _mqttClient(mqttClient) {};
    uint8_t create_device(HADiscoveryPayload ha_discovery_payload);
};
#endif
