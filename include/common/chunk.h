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
};

class EspNowChunker {
   public:
    void send(uint8_t* destinationMac, uint8_t* payload, size_t len) {
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
            chunk.totalChunks = totalChunks;
            chunk.chunkIndex = ptr - payload;
            chunk.len = currentChunkSize;
            chunk.totalLen = len;
            memcpy(&chunk.data, ptr, currentChunkSize);

            esp_now_send(destinationMac, (uint8_t*)&chunk, sizeof(chunk));
        }
    }
};
#endif
