#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H
#include <Arduino.h>
#include <ArduinoJson.h>

#include "constants.h"
#include "serializable.h"

/**
 * For more information about the definitions of terms,
 * visit the HomeAssitant MQTT integration documentation.
 * https://www.home-assistant.io/integrations/mqtt/
 */

/**
 * Message type enum, used to specify the types of
 * message being sent over UART, allowing the receiver
 * to read it.
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
 * Value type enum used to specify the type of data
 * being provided in HAStateUpdate structs.
 */
enum class ValueType : uint8_t { INT, FLOAT, BOOL, CHAR_128 };

/**
 * Represents a device in Home Assistant's MQTT integration
 */
struct HADevice {
    MessageType type = MessageType::DEVICE;
    char ids[17];    // deviceid -- uuid is typically 16 bytes long
    char name[256];  // name
    char mf[256];    // manufacturer
    char mdl[32];    // model
};

/**
 * Represents a component in Home Assistant's MQTT integration.
 */
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

/**
 * Represents an origin in Home Assistant's MQTT integration.
 */
struct HAOrigin {
    MessageType type = MessageType::ORIGIN;
    char name[256];  // name
    char sw[32];     // sw_version
    char url[256];   // support_url

    JsonDocument toJSON();
};

/**
 * Represents a key-value pair for a component in
 * Home Assistant's autodiscovery message
 */
struct HAComponent {
    const char* key;
    HAComponentOptions value;
};

/**
 * Represents a payload that is sent to Home Assistant's
 * MQTT integration for automatic discovery of devices.
 */
struct HADiscoveryPayload {
    MessageType type = MessageType::DISCOVERY_PAYLOAD;
    HADevice* dev = nullptr;
    HAOrigin* origin = nullptr;
    HAComponent* cmps = nullptr;  // pointer to the start of the cmps array
    size_t cmpCount;              // count of cmps in the array

    JsonDocument toJSON();
};

/**
 * Represents an update to a value or reading that a
 * sensor wants to send to Home Assistant
 */
template <typename T>
struct HAStateTraits;

template <>
struct HAStateTraits<float> {
    static constexpr MessageType messageType = MessageType::STATE_UPDATE_FLOAT;
};

template <>
struct HAStateTraits<int> {
    static constexpr MessageType messageType = MessageType::STATE_UPDATE_INT;
};

template <>
struct HAStateTraits<char[128]> {
    static constexpr MessageType messageType =
        MessageType::STATE_UPDATE_CHAR_128;
};

template <typename T>
struct HAStateUpdate : Serializable {
    MessageType messageType = HAStateTraits<T>::messageType;
    char topic[128];  // MQTT topic to publish state to
    T value;

    /**
     * @brief Writes the item to the given serial connection.
     * @param serial A HardwareSerial instance to write to.
     * @return Indicator or success or failure
     */
    uint8_t write(HardwareSerial& serial) override {
        serial.write(MESSAGE_START);
        serial.write((uint8_t)this->messageType);
        serial.write((uint8_t*)this, sizeof(*this));
        return 1;
    }
};
#endif
