#include "ha_manager.h"

#include <ArduinoJson.h>

#include "ArduinoJson/Document/JsonDocument.hpp"
#include "config.h"
#include "message_type.h"
#include "mqtt_client.h"

/**
 * @brief Publishes a discovery message to Home Assistant's MQTT integration
 * @param discoveryPayload The discovery payload to send to HA
 * @return a 1 indicating success, 0 indicating failure.
 */
uint8_t HAManager::discovery(HADiscoveryPayload discoveryPayload) {
    JsonDocument jsonPayload = discoveryPayload.toJSON();

    char topic[255];
    snprintf(topic, sizeof(topic), "%s/device/%s/config", HA_DISCOVERY_PREFIX,
             discoveryPayload.dev->ids);

    // Create buffer of the size required to send the message
    size_t requiredSize = measureJson(jsonPayload);

    if (requiredSize > 1024) {
        Serial.println(
            "createDevice - jsonPayload too large! Maximum of 1024 bytes for "
            "MQTT "
            "PubSubClient");
        return 0;
    }

    /**
     * Allocate buffer to store the serialized JSON
     * payload before sending over MQTT. Add 1 for
     * null terminator
     */
    char* heapBuffer = new char[requiredSize + 1];

    if (heapBuffer != nullptr) {
        /**
         * Not adding null-terminator since the heapBuffer size
         * is calculated based on the JSON payload. serializeJson
         * should have room to null terminate
         */
        serializeJson(jsonPayload, heapBuffer, requiredSize);
        heapBuffer[requiredSize] = '\0';

        Serial.printf("discovery - Sending message: \n");
        serializeJsonPretty(jsonPayload, Serial);
        Serial.println();

        /**
         * TODO: We will need to either publish the discovery message
         * with the retain flag `this->_mqttClient.publish(topic, heapBuffer,
         * true)` or subscribe to a specific topic from the device and republish
         * when the mqtt broker comes back on
         */
        this->_mqttClient.publish(topic, heapBuffer);
        delete[] heapBuffer;

        return 1;
    }

    Serial.println(
        "discovery - Failed to allocate memory for serializing JSON "
        "payload");
    return 0;
}

/**
 * @brief Publishes a state update to Home Assistant
 * @param stateUpdate The state update message to publish.
 * @return a 1 indicating success, 0 indicating failure.
 */
uint8_t HAManager::publishState(HAStateUpdate<float> stateUpdate) {
    char strValue[64];

    if (stateUpdate.valueType != ValueType::FLOAT_TYPE) {
        Serial.println(
            "publishState - function for float type called but state update is "
            "not a float.");
        return 0;
    }

    snprintf(strValue, sizeof(strValue), "%.4f", stateUpdate.value);
    Serial.printf("Publishing state %s to topic %s\n", strValue,
                  stateUpdate.topic);

    this->_mqttClient.publish(stateUpdate.topic, strValue);
    return 1;
}

uint8_t HAManager::publishState(HAStateUpdate<int32_t> stateUpdate) {
    char strValue[64];

    if (stateUpdate.valueType != ValueType::INT_TYPE) {
        Serial.println(
            "publishState - function for int type called but state update is "
            "not an int.");
        return 0;
    }

    snprintf(strValue, sizeof(strValue), "%ud", stateUpdate.value);
    Serial.printf("Publishing state %s to topic %s\n", strValue,
                  stateUpdate.topic);

    this->_mqttClient.publish(stateUpdate.topic, strValue);
    return 1;
}

uint8_t HAManager::publishState(HAStateUpdate<bool> stateUpdate) {
    if (stateUpdate.valueType != ValueType::BOOL_TYPE) {
        Serial.println(
            "publishState - function for bool type called but state update is "
            "not an bool.");
        return 0;
    }

    char strValue[4];

    if (stateUpdate.value) {
        strncpy(strValue, "OFF", sizeof(strValue));
    } else {
        strncpy(strValue, "ON", sizeof(strValue));
    }
    strValue[sizeof(strValue) - 1] = '\0';

    Serial.printf("Publishing state %s to topic %s\n", strValue,
                  stateUpdate.topic);

    this->_mqttClient.publish(stateUpdate.topic, strValue);
    return 1;
}
