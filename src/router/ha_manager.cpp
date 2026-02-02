#include "ha_manager.h"

#include <ArduinoJson.h>

#include "ArduinoJson/Document/JsonDocument.hpp"
#include "ArduinoJson/Object/JsonObject.hpp"
#include "mqtt_client.h"

uint8_t HAManager::create_device(HADiscoveryPayload haDiscoveryPayload) {
    JsonDocument jsonPayload;

    JsonObject dev = jsonPayload["dev"].to<JsonObject>();
    dev["ids"] = haDiscoveryPayload.dev->ids;
    dev["name"] = haDiscoveryPayload.dev->name;
    dev["mf"] = haDiscoveryPayload.dev->mf;
    dev["mdl"] = haDiscoveryPayload.dev->mdl;

    if (haDiscoveryPayload.origin != nullptr) {
        JsonObject o = jsonPayload["o"].to<JsonObject>();
        o["name"] = haDiscoveryPayload.origin->name;
        o["sw"] = haDiscoveryPayload.origin->sw;
        o["url"] = haDiscoveryPayload.origin->url;
    }

    if (haDiscoveryPayload.cmps != nullptr) {
        JsonObject cmps = jsonPayload["cmps"].to<JsonObject>();

        for (size_t i = 0; i < haDiscoveryPayload.cmpCount; i++) {
            HAComponent component = haDiscoveryPayload.cmps[i];

            const char* key = component.key;
            HAComponentOptions value = component.value;

            JsonObject currentObject = cmps[key].to<JsonObject>();
            currentObject["p"] = value.p;
            currentObject["dev_cla"] = value.dev_cla;
            currentObject["uniq_id"] = value.uniq_id;
            currentObject["stat_t"] = value.stat_t;
            currentObject["unit_of_meas"] = value.unit_of_meas;
        }
    }

    const char* topic = "homeassistant/device/somedevice1234/config";

    size_t requiredSize = measureJson(jsonPayload);

    size_t bufferSize = requiredSize + 16;

    char* heapBuffer = new char[bufferSize];

    if (heapBuffer != nullptr) {
        serializeJson(jsonPayload, heapBuffer, bufferSize);
        heapBuffer[bufferSize - 1] =
            '\0';  // Ensure null-terminated, even if we overflow

        Serial.printf("create_device - Sending message: \n");
        serializeJsonPretty(jsonPayload, Serial);
        Serial.println();

        this->_mqttClient.publish(topic, heapBuffer);
        delete[] heapBuffer;

        return 1;
    }

    Serial.println(
        "create_device - Failed to allocate memory for serializing JSON "
        "payload");
    return 0;
}
