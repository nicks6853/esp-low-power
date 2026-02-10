#include "message_type.h"

#include <ArduinoJson.h>

/**
 * @brief Converts the object to a JSON document.
 * @return The JSON document (smart pointer)
 */
JsonDocument HADiscoveryPayload::toJSON() {
    JsonDocument jsonPayload;

    JsonObject dev = jsonPayload["dev"].to<JsonObject>();
    dev["ids"] = this->dev.ids;
    dev["name"] = this->dev.name;
    dev["mf"] = this->dev.mf;
    dev["mdl"] = this->dev.mdl;

    // TODO: Make this->origin optional in HADiscoveryPayload
    jsonPayload["o"] = this->origin.toJSON();

    // If there are components, convert them to JSON
    if (this->cmpCount > 0) {
        JsonObject cmps = jsonPayload["cmps"].to<JsonObject>();

        for (size_t i = 0; i < this->cmpCount; i++) {
            HAComponent component = this->cmps[i];

            const char* key = component.key;
            HAComponentOptions value = component.value;

            cmps[key] = value.toJSON();
        }
    }

    return jsonPayload;
}

/**
 * @brief Converts the object to a JSON document.
 * @return The JSON document (smart pointer)
 */
JsonDocument HAOrigin::toJSON() {
    JsonDocument jsonPayload;
    jsonPayload["name"] = this->name;
    jsonPayload["sw"] = this->sw;
    jsonPayload["url"] = this->url;

    return jsonPayload;
}

/**
 * @brief Converts the object to a JSON document.
 * @return The JSON document (smart pointer)
 */
JsonDocument HAComponentOptions::toJSON() {
    JsonDocument jsonPayload;
    jsonPayload["p"] = this->p;
    jsonPayload["dev_cla"] = this->dev_cla;
    jsonPayload["uniq_id"] = this->uniq_id;
    jsonPayload["stat_t"] = this->stat_t;
    jsonPayload["unit_of_meas"] = this->unit_of_meas;
    jsonPayload["name"] = this->name;

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
