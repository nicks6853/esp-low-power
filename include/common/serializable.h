#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H
#include <Arduino.h>
#include <HardwareSerial.h>

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

struct Serializable {
    MessageType messageType;
    virtual ~Serializable() = default;
    virtual void write(HardwareSerial& serial) const = 0;
};

#endif
