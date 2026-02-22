#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H
#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"

/**
 * For more information about the definitions of terms,
 * visit the HomeAssitant MQTT integration documentation.
 * https://www.home-assistant.io/integrations/mqtt/
 */

/**
 * Value type enum used to specify the type of data
 * being provided in HAStateUpdate structs.
 */
enum class ValueType : uint8_t { INT, FLOAT, BOOL, CHAR_128 };

/**
 * Different message types that a HAMessage
 * struct can contain.
 */
enum class MessageType : uint8_t {
    DEVICE,
    COMPONENT_OPTIONS,
    ORIGIN,
    DISCOVERY_PAYLOAD,
    STATE_UPDATE_FLOAT,
    STATE_UPDATE_INT,
    STATE_UPDATE_BOOL,
    STATE_UPDATE_CHAR_128,
};

/**
 * Represents a device in Home Assistant's MQTT integration
 */
struct HADevice {
    char ids[17];    // deviceid -- uuid is typically 16 bytes long
    char name[256];  // name
    char mf[256];    // manufacturer
    char mdl[32];    // model
};

/**
 * Represents a component in Home Assistant's MQTT integration.
 */
struct HAComponentOptions {
    char p[32];             // platform
    char dev_cla[32];       // device_class
    char uniq_id[32];       // unique_id -- uuid is typically 16 bytes long
    char stat_t[32];        // state_topic
    char unit_of_meas[32];  // unit_of_measurement
    char name[32];          // name
};

/**
 * Represents an origin in Home Assistant's MQTT integration.
 */
struct HAOrigin {
    char name[256];  // name
    char sw[32];     // sw_version
    char url[256];   // support_url
};

/**
 * Represents a key-value pair for a component in
 * Home Assistant's autodiscovery message
 */
struct HAComponent {
    char key[32];
    HAComponentOptions value;
};

/**
 * Represents a payload that is sent to Home Assistant's
 * MQTT integration for automatic discovery of devices.
 */
struct HADiscoveryPayload {
    HADevice dev;
    HAOrigin origin;
    HAComponent cmps[HA_MAX_COMPONENT_PER_DEVICE];  // Array of the components
    size_t cmpCount;  // count of cmps in the array
};

/**
 * Represents an update to a value or reading that a
 * sensor wants to send to Home Assistant
 */
template <typename T>
struct HAStateUpdate {
    char topic[128];  // MQTT topic to publish state to
    T value;
};

/**
 * Struct to represent a message that is sent from an edge device
 * through the system.
 */
struct HAMessage {
    MessageType messageType;
    union {
        HAStateUpdate<float> stateUpdateF;
        HAStateUpdate<int> stateUpdateI;
        HAStateUpdate<char[128]> stateUpdateS;
        HADiscoveryPayload discovery;
    } payload;
};
#endif
