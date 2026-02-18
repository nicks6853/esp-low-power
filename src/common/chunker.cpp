#include "chunker.h"

/**
 * @brief Sends a payload chunk by chunk to the given destination MAC address.
 * @param destinationMac The MAC address of the ESPNOW recipient.
 * @param payload The bytes to send over ESPNOW.
 * @param len The length of the byte array to send over ESPNOW.
 */
void EspNowChunker::send(uint8_t* destinationMac, uint8_t* payload,
                         size_t len) {
    uint8_t checksum = this->_calculateChecksum(payload, len);
    size_t currentChunkSize;
    size_t remainingSize;
    uint8_t* end = payload + len;
    size_t totalChunks =
        (len + CHUNK_SIZE - 1) /
        CHUNK_SIZE;  // Amount of chunks required to send full payload

    for (uint8_t* ptr = payload; ptr < end; ptr += CHUNK_SIZE) {
        remainingSize = (size_t)(end - ptr);
        currentChunkSize =
            remainingSize < CHUNK_SIZE ? remainingSize : CHUNK_SIZE;

        EspNowChunk chunk;
#if defined(ESP32)
        chunk.chipId = ESP.getEfuseMac();
#elif defined(ESP8266)
        chunk.chipId = ESP.getChipId();
#endif
        chunk.checksum = checksum;
        chunk.totalChunks = totalChunks;
        chunk.chunkIndex = ptr - payload;
        chunk.len = currentChunkSize;
        chunk.totalLen = len;
        memcpy(&chunk.data, ptr, currentChunkSize);

        esp_now_send(destinationMac, (uint8_t*)&chunk, sizeof(chunk));
    }
}

/**
 * @brief Calculates a basic XOR checksum for a byte array
 * @param buffer The byte array to calculate a checksum for.
 * @param len The length of the byte array.
 * @return The XOS checksum for the given byte array.
 */
uint8_t EspNowChunker::_calculateChecksum(const uint8_t* buffer, size_t len) {
    uint8_t checksum = 0;

    for (size_t i = 0; i < len; i++) {
        checksum ^= buffer[i];
    }

    return checksum;
}
