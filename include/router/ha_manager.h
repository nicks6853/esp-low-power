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
    uint8_t discovery(HADiscoveryPayload discoveryPayload);

    uint8_t publishStateUpdate(HAStateUpdate<float> stateUpdate);
    uint8_t publishStateUpdate(HAStateUpdate<bool> stateUpdate);
    uint8_t publishStateUpdate(HAStateUpdate<int32_t> stateUpdate);
};
#endif
