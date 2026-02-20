#ifndef RECEIVERTASK_H
#define RECEIVERTASK_H
#include <Arduino.h>

#include "chunker.h"
#include "message_type.h"

#define MAX_ACTIVE_BUFFER_COUNT 5
#define PAYLOAD_TIMEOUT 200
#define MAX_CALLBACK_FUNCTIONS 1

// Forward declaration for structs
class ReceiverTask;
struct ActiveBuffer;

/**
 * Struct for data that is given to the timers
 * that clear active buffers after they've been
 * inactive for too long
 */
struct RequestTimeoutData {
    ReceiverTask* instance;
    TimerHandle_t timer;
};

/**
 * Struct representing one of the active buffers that is
 * recording chunks from messages coming from edge devices.
 */
struct ActiveBuffer {
    uint64_t chipId = 0;
    uint8_t* buffer = nullptr;
    size_t chunksRead = 0;
    size_t bytesRead = 0;
    RequestTimeoutData* timeoutData;
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

/**
 * Function type for callback called when a complete message
 * is pieced together from chunks.
 */
typedef void (*onCompleteCallback)(const HAMessage* incomingMsg);

class ReceiverTask {
   private:
    QueueHandle_t _procQueue;
    TaskHandle_t _task;
    ActiveBuffer _activeBuffers[MAX_ACTIVE_BUFFER_COUNT];
    onCompleteCallback _callbacks[MAX_CALLBACK_FUNCTIONS];
    size_t _registeredCallbacksCount = 0;

    HAMessage* _handleChunk(EspNowChunk& chunk);
    ActiveBuffer* _findActiveBuffer(uint64_t chipId);
    void _resetActiveBuffer(ActiveBuffer* buffer, bool free = true);
    static void _requestTimeoutCallback(TimerHandle_t xTimer);
    static void _taskBody(void* pvParameters);

   public:
    ReceiverTask(const char* name, uint16_t stackSize, UBaseType_t priority);
    void pushMsg(esp_now_message_t msg);
    void registerCompleteCallback(onCompleteCallback func);
};
#endif
