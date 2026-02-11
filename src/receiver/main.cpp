/**
 * Notes:
 * Clean up code in the queue processing function
 * Add a checksum on the chunks from the edge device, and verify the complete
 * payload once it's been received. As messages are complete on the queue,
 * forward them to the Serial connection.
 *
 */
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#include "chunk.h"
#include "config.h"
#include "message_type.h"

struct EspNowState {
    uint8_t channel;
};

EspNowState espnowState;
HAMessage* currentMessage;
QueueHandle_t msgQueue;

unsigned long lastAction = 0;
unsigned long currentTime;

struct esp_now_message_t {
    uint8_t mac_addr[6];
    uint8_t payload[250];  // ESP-NOW max is 250 bytes
    size_t length;
};

struct ActiveBuffer {
    uint64_t chipId;
    uint8_t* buffer = nullptr;
    size_t chunksRead = 0;
    unsigned long readStart = 0;
};

/**
 * Receives message chunks from ESPNOW and pushes it to a queue to be processed.
 * @param mac The mac address of the sender
 * @param incomingData The bytes of data received from the sender
 * @param len The length of the incoming data.
 */
void receiveCallback(const uint8_t* mac, const uint8_t* incomingData, int len) {
    Serial.println("Received a message!");

    esp_now_message_t msg;
    memcpy(msg.mac_addr, mac, 6);
    memcpy(msg.payload, incomingData, len);
    msg.length = len;

    if (xQueueSend(msgQueue, &msg, 0) != pdTRUE) {
        Serial.println("Queue is full! Unable to process message");
    }
}

/**
 * Background process to read chunks coming from ESPNOW
 * that are on the queue and build messages out of them.
 * @param pvParameters Parameters from the PV
 */
void processTask(void* pvParameters) {
    esp_now_message_t receivedMsg;

    // Initialize all the potential active buffers to 0 / nullptr
    ActiveBuffer activeBuffers[10];
    for (size_t i = 0; i < sizeof(activeBuffers) / sizeof(ActiveBuffer); i++) {
        activeBuffers[i].buffer = nullptr;
        activeBuffers[i].chipId = 0;
    }

    while (true) {
        // wait indefinitely for a message (portMAX_DELAY)
        if (xQueueReceive(msgQueue, &receivedMsg, portMAX_DELAY) == pdTRUE) {
            Serial.println("Processing queue message");
            EspNowChunk currentChunk;
            memcpy(&currentChunk, receivedMsg.payload, receivedMsg.length);

            // Find the active buffer for this chunk based on chipId
            ActiveBuffer* activeBuffer = nullptr;
            ActiveBuffer* firstAvailableBuffer = nullptr;
            for (size_t i = 0;
                 i < sizeof(activeBuffers) / sizeof(ActiveBuffer) &&
                 activeBuffer == nullptr;
                 i++) {
                if (activeBuffers[i].chipId == currentChunk.chipId) {
                    // Save a pointer to the active buffer
                    activeBuffer = &activeBuffers[i];
                }

                if (firstAvailableBuffer == nullptr &&
                    activeBuffers[i].chipId == 0) {
                    firstAvailableBuffer = &activeBuffers[i];
                }
            }

            // If we did not find an available buffer, use
            // the first buffer in the activeBuffers array that
            // was not currently in use.
            if (activeBuffer == nullptr) {
                activeBuffer = firstAvailableBuffer;
            }

            if (activeBuffer->buffer == nullptr) {
                if (currentChunk.chunkIndex != 0) {
                    Serial.println(
                        "Received non-first chunk without active buffer, "
                        "ignoring");
                    continue;
                }
                Serial.println("Reading a new payload");
                // We are processing a new chunk
                activeBuffer->chipId =
                    currentChunk.chipId;  // Mark the buffer with the chip ID
                activeBuffer->buffer = new uint8_t[currentChunk.totalLen];
                activeBuffer->readStart = millis();
                activeBuffer->chunksRead = 0;
            }

            if (activeBuffer->chunksRead > currentChunk.totalChunks) {
                Serial.println("Received too many bytes! Clearing the buffer");
                activeBuffer->chunksRead = 0;
                delete[] activeBuffer->buffer;
                activeBuffer->buffer = nullptr;
                activeBuffer->chipId = 0;
                activeBuffer->readStart = 0;
                continue;
            }

            if (millis() - activeBuffer->readStart > 200) {
                Serial.println(
                    "Payload took longer than 500ms to come in, aborting.");
                activeBuffer->chunksRead = 0;
                delete[] activeBuffer->buffer;
                activeBuffer->buffer = nullptr;
                activeBuffer->chipId = 0;
                activeBuffer->readStart = 0;
                continue;
            }

            memcpy(activeBuffer->buffer + currentChunk.chunkIndex,
                   &currentChunk.data, currentChunk.len);
            activeBuffer->chunksRead += 1;

            if (activeBuffer->chunksRead == currentChunk.totalChunks) {
                Serial.println("Finished reading payload");
                HAMessage* msg = (HAMessage*)activeBuffer->buffer;
                Serial.println("===================");
                Serial.printf("Message type: %d, topic: %s, value: %.2f\n",
                              (uint8_t)msg->messageType,
                              msg->stateUpdateF.topic, msg->stateUpdateF.value);
                Serial.println("===================");
                activeBuffer->chunksRead = 0;
                delete[] activeBuffer->buffer;
                activeBuffer->buffer = nullptr;
                activeBuffer->chipId = 0;
                activeBuffer->readStart = 0;
            }
        }
    }
}

uint8_t initializeEspNow() {
    Serial.println("Initializing ESPNOW...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Print mac address
    Serial.print("Mac address: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESPNOW!");
        return 0;
    }

    Serial.println("ESPNOW Initialized!");

    return 1;
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);

    while (!Serial);

    Serial.println("Serial is ready!");

    msgQueue = xQueueCreate(20, sizeof(esp_now_message_t));
    xTaskCreate(processTask, "ProcessTask", 4096, NULL, 1, NULL);

    initializeEspNow();
    uint8_t res = esp_now_register_recv_cb(receiveCallback);
    Serial.printf("RESULT FROM REGISTERING CALLBACK: %d", res);
}

void loop() {
    currentTime = millis();
    if (currentTime - lastAction > 2000) {
        Serial.println(ESP.getFreeHeap());
        lastAction = currentTime;
    }
}
