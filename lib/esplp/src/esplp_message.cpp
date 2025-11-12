#include "esplp_message.h"

/**
 * ESPLPDiscovery Implementation
 */
void ESPLPDiscovery::serialize(uint8_t* buffer) const {
    memcpy(buffer, this->mac, sizeof(this->mac));
    memcpy(buffer + sizeof(this->mac), this->name, sizeof(this->name));
}

void ESPLPDiscovery::deserialize(const uint8_t* buffer) {
    memcpy(&this->mac, buffer, sizeof(this->mac));
    memcpy(&this->name, buffer + sizeof(this->mac), sizeof(this->name));
}

/**
 * ESPLPAutopair Implementation
 */
void ESPLPAutopair::serialize(uint8_t* buffer) const {
    buffer[0] = this->wifiChannel;
    memcpy(buffer + sizeof(this->wifiChannel), this->mac, sizeof(this->mac));
}

void ESPLPAutopair::deserialize(const uint8_t* buffer) {
    this->wifiChannel = buffer[0];
    memcpy(&this->mac, buffer + 1, sizeof(this->mac));
}

/**
 * ESPLPMessage Implementation
 */
void ESPLPMessage::serialize(uint8_t* buffer) const {
    buffer[0] = this->type;
    buffer[1] = this->size;
    memcpy(buffer + 2, this->data, sizeof(this->data));
}

void ESPLPMessage::deserialize(const uint8_t* buffer) {
    this->type = (ESPLPMessageType)buffer[0];
    this->size = buffer[1];
    memcpy(this->data, buffer + 2, sizeof(this->data));
}
