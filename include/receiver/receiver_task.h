#ifndef RECEIVERTASK_H
#define RECEIVERTASK_H
#include <Arduino.h>

#include "chunker.h"
#include "config.h"
#include "message_type.h"

#define MAX_ACTIVE_BUFFER_COUNT 5
#define PAYLOAD_TIMEOUT 200

#ifdef DEBUG
#define LOG(x) x
#else
#define LOG(X) (void)0
#endif

/**
 * Struct representing one of the active buffers that is
 * recording chunks from messages coming from edge devices.
 */
struct ActiveBuffer {
    uint64_t chipId = 0;
    uint8_t* buffer = nullptr;
    size_t chunksRead = 0;
    size_t bytesRead = 0;
    unsigned long readStart = 0;
};

/**
 * Represents a messages received on ESPNOW
 * that is queued to be processed.
 */
struct esp_now_message_t {
    uint8_t mac_addr[6];
    uint8_t payload[250];  // ESP-NOW max is 250 bytes
    size_t length;
};

class ReceiverTask {
   private:
    QueueHandle_t _procQueue;
    TaskHandle_t _task;
    ActiveBuffer _activeBuffers[MAX_ACTIVE_BUFFER_COUNT];

    HAMessage* _handleChunk(EspNowChunk& chunk);
    ActiveBuffer* _findActiveBuffer(uint64_t chipId);
    void _resetActiveBuffer(ActiveBuffer* buffer, bool free = true);

   public:
    ReceiverTask(const char* name, uint16_t stackSize, UBaseType_t priority);
    static void taskBody(void* pvParameters);
    void pushMsg(esp_now_message_t msg);
};
#endif
