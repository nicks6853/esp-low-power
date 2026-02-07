#ifndef EDGEMESSAGE_H
#define EDGEMESSAGE_H

#include <Arduino.h>

enum ESPLPMessageType {
    AUTOPAIR,
    DISCOVERY,
    UPDATE,
};

/**
 * Base class for all messages coming from an edge device.
 */
struct ESPLPMessageBase {
    /**
     * Serialize the message data into the given buffer
     * @param buffer The buffer to copy the data to
     */
    virtual void serialize(uint8_t* buffer) const = 0;

    /**
     * Deserialize the data from the given buffer
     * @param buffer The buffer to retrieve serialized data
     *  from.
     */
    virtual void deserialize(const uint8_t* buffer) = 0;
};

/**
 * Message class for an edge device autopair message.
 */
struct ESPLPAutopair : ESPLPMessageBase {
    uint8_t wifiChannel;  // Current Wifi channel
    uint8_t mac[6];  // Mac address, format {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}

    /**
     * Returns the size of this message type in bytes
     * @return The size of this message type in bytes
     */
    static constexpr size_t size() { return sizeof(mac) + sizeof(wifiChannel); }

    void serialize(uint8_t* buffer) const;
    void deserialize(const uint8_t* buffer);
};

/**
 * Message class for an edge device discovery message.
 */
struct ESPLPDiscovery : ESPLPMessageBase {
    uint8_t mac[6];  // Mac address, format {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}
    char name[33];   // Max 32 characters

    /**
     * Returns the size of this message type in bytes
     * @return The size of this message type in bytes
     */
    static constexpr size_t size() { return sizeof(mac) + sizeof(name); }

    void serialize(uint8_t* buffer) const;
    void deserialize(const uint8_t* buffer);
};

/**
 * Calculate the max message size at compile time
 */
template <size_t A, size_t B>
struct Max {
    static constexpr size_t value = (A > B) ? A : B;
};
constexpr size_t MAX_MESSAGE_SIZE =
    Max<ESPLPDiscovery::size(), ESPLPAutopair::size()>::value;

/**
 * Define the ESPLPMessage type which is the message type that we will
 * be sending out.
 */
struct ESPLPMessage {
    ESPLPMessageType type;
    size_t size;
    uint8_t data[MAX_MESSAGE_SIZE];

    void serialize(uint8_t* buffer) const;
    void deserialize(const uint8_t* buffer);
};

#endif
