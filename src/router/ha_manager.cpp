#include "ha_manager.h"

#include <ArduinoJson.h>

#include "ArduinoJson/Document/JsonDocument.hpp"
#include "config.h"
#include "message_type.h"
#include "mqtt_client.h"

/**
 * Forward declarations
 */
JsonDocument toJSON(HAOrigin item);
JsonDocument toJSON(HADiscoveryPayload item);
JsonDocument toJSON(HAComponentOptions item);

/**
 * @brief Converts the object to a JSON document.
 * @param item The HADiscoveryPayload to convert to JSON
 * @return The JSON document (smart pointer)
 */
JsonDocument toJSON(HADiscoveryPayload item) {
    JsonDocument jsonPayload;

    JsonObject dev = jsonPayload["dev"].to<JsonObject>();
    dev["ids"] = item.dev.ids;
    dev["name"] = item.dev.name;
    dev["mf"] = item.dev.mf;
    dev["mdl"] = item.dev.mdl;

    // TODO: Make item.origin optional in HADiscoveryPayload
    jsonPayload["o"] = toJSON(item.origin);

    // If there are components, convert them to JSON
    if (item.cmpCount > 0) {
        JsonObject cmps = jsonPayload["cmps"].to<JsonObject>();

        for (size_t i = 0; i < item.cmpCount; i++) {
            HAComponent component = item.cmps[i];

            const char* key = component.key;
            HAComponentOptions value = component.value;

            cmps[key] = toJSON(value);
        }
    }

    return jsonPayload;
}

/**
 * @brief Converts the object to a JSON document.
 * @param item The HAOrigin to convert to JSON
 * @return The JSON document (smart pointer)
 */
JsonDocument toJSON(HAOrigin item) {
    JsonDocument jsonPayload;
    jsonPayload["name"] = item.name;
    jsonPayload["sw"] = item.sw;
    jsonPayload["url"] = item.url;

    return jsonPayload;
}

/**
 * @brief Converts the object to a JSON document.
 * @param item The HAComponentOptions to convert to JSON
 * @return The JSON document (smart pointer)
 */
JsonDocument toJSON(HAComponentOptions item) {
    JsonDocument jsonPayload;
    jsonPayload["p"] = item.p;
    jsonPayload["dev_cla"] = item.dev_cla;
    jsonPayload["uniq_id"] = item.uniq_id;
    jsonPayload["stat_t"] = item.stat_t;
    jsonPayload["unit_of_meas"] = item.unit_of_meas;
    jsonPayload["name"] = item.name;

    return jsonPayload;
}

/**
 * @brief Publishes a discovery message to Home Assistant's MQTT integration
 * @param discoveryPayload The discovery payload to send to HA
 * @return a 1 indicating success, 0 indicating failure.
 */
uint8_t HAManager::discovery(HADiscoveryPayload discoveryPayload) {
    JsonDocument jsonPayload = toJSON(discoveryPayload);

    char topic[255];
    snprintf(topic, sizeof(topic), "%s/device/%s/config", HA_DISCOVERY_PREFIX,
             discoveryPayload.dev.ids);

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
uint8_t HAManager::publishStateUpdate(HAStateUpdate<float> stateUpdate) {
    char strValue[64];

    snprintf(strValue, sizeof(strValue), "%.4f", stateUpdate.value);
    Serial.printf("Publishing state %s to topic %s\n", strValue,
                  stateUpdate.topic);

    this->_mqttClient.publish(stateUpdate.topic, strValue);
    return 1;
}

uint8_t HAManager::publishStateUpdate(HAStateUpdate<int32_t> stateUpdate) {
    char strValue[64];

    snprintf(strValue, sizeof(strValue), "%ud", stateUpdate.value);
    Serial.printf("Publishing state %s to topic %s\n", strValue,
                  stateUpdate.topic);

    this->_mqttClient.publish(stateUpdate.topic, strValue);
    return 1;
}

uint8_t HAManager::publishStateUpdate(HAStateUpdate<bool> stateUpdate) {
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
