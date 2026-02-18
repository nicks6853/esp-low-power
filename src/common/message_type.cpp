#include "message_type.h"

#include <ArduinoJson.h>

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

HAMessage::HAMessage(MessageType msgType) : messageType(msgType) {
    switch (this->messageType) {
        case MessageType::STATE_UPDATE_FLOAT:
            this->stateUpdateF = HAStateUpdate<float>{};
            break;
        case MessageType::STATE_UPDATE_INT:
            this->stateUpdateI = HAStateUpdate<int>{};
            break;
        case MessageType::STATE_UPDATE_CHAR_128:
            this->stateUpdateS = HAStateUpdate<char[128]>{};
            break;
        case MessageType::DISCOVERY_PAYLOAD:
            this->discovery = HADiscoveryPayload{};
            break;
        default:
            Serial.println("Unknown message type in HAMessage constructor");
            break;
    }
}

/**
 * @brief Destructor for the HAMessage struct
 */
HAMessage::~HAMessage() {
    switch (this->messageType) {
        case MessageType::DISCOVERY_PAYLOAD: {
            this->discovery.~HADiscoveryPayload();
            break;
        }
        default:
            break;
    }
}
