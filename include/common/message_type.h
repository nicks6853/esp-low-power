#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H
#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * For more information about the definitions of terms,
 * visit the HomeAssitant MQTT integration documentation.
 * https://www.home-assistant.io/integrations/mqtt/
 */

enum class MessageType : uint8_t {
    DEVICE,
    COMPONENT_OPTIONS,
    ORIGIN,
    DISCOVERY_PAYLOAD,
    STATE_UPDATE,
};

enum class ValueType : uint8_t {
    INT_TYPE = 0,
    FLOAT_TYPE = 1,
    BOOL_TYPE = 2,
};

struct HADevice {
    MessageType type = MessageType::DEVICE;
    char ids[17];    // deviceid -- uuid is typically 16 bytes long
    char name[256];  // name
    char mf[256];    // manufacturer
    char mdl[32];    // model
};

struct HAComponentOptions {
    MessageType type = MessageType::COMPONENT_OPTIONS;
    char p[32];             // platform
    char dev_cla[32];       // device_class
    char uniq_id[32];       // unique_id -- uuid is typically 16 bytes long
    char stat_t[32];        // state_topic
    char unit_of_meas[32];  // unit_of_measurement
    char name[32];          // name

    JsonDocument toJSON();
};

struct HAOrigin {
    MessageType type = MessageType::ORIGIN;
    char name[256];  // name
    char sw[32];     // sw_version
    char url[256];   // support_url

    JsonDocument toJSON();
};

struct HAComponent {
    const char* key;
    HAComponentOptions value;
};

struct HADiscoveryPayload {
    MessageType type = MessageType::DISCOVERY_PAYLOAD;
    HADevice* dev = nullptr;
    HAOrigin* origin = nullptr;
    HAComponent* cmps = nullptr;  // pointer to the start of the cmps array
    size_t cmpCount;              // count of cmps in the array

    JsonDocument toJSON();
};

template <typename T>
struct HAStateUpdate {
    MessageType type = MessageType::STATE_UPDATE;
    char topic[128];  // Topic to publish state to
    T value;
    ValueType valueType;  // Type of the value that is used
};

#endif
