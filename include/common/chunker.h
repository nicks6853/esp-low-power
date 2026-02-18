#ifndef ESPNOWCHUNKER_H
#define ESPNOWCHUNKER_H
#include <Arduino.h>

#if defined(ESP32)
#include <esp_now.h>
#elif defined(ESP8266)
#include <espnow.h>
#endif

constexpr uint8_t CHUNK_SIZE =
    230;  // Needs to be less than 233 to send metadata with the chunks

struct EspNowChunk {
    uint64_t chipId;  // The ID of the chip sending the chunk
    uint16_t totalChunks;
    uint16_t chunkIndex;
    uint16_t len;       // Size of the current chunk
    uint16_t totalLen;  // Size of the entire payload
    uint8_t data[CHUNK_SIZE];
    uint8_t checksum;
};

class EspNowChunker {
   private:
    uint8_t _calculateChecksum(const uint8_t* buffer, size_t len);

   public:
    void send(uint8_t* destinationMac, uint8_t* payload, size_t len);
};
#endif
