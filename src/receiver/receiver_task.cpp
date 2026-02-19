#include "receiver_task.h"

ReceiverTask::ReceiverTask(const char* name, uint16_t stackSize,
                           UBaseType_t priority) {
    // Initialize the queue
    this->_procQueue = xQueueCreate(20, sizeof(esp_now_message_t));

    if (this->_procQueue == NULL) {
        LOG(Serial.println("Failed to created _procQueue"));
    }

    BaseType_t result = xTaskCreate(this->taskBody, name, stackSize,
                                    (void*)this, priority, &this->_task);

    if (result != pdPASS) {
        LOG(Serial.println("Failed to create _task"));
        this->_task = NULL;
    }
}

/**
 * Main body of the task
 * @param pvParameters The parameters given to the task
 */
void ReceiverTask::taskBody(void* pvParameters) {
    ReceiverTask* instance = (ReceiverTask*)pvParameters;
    esp_now_message_t receivedMsg;

    while (true) {
        if (xQueueReceive(instance->_procQueue, &receivedMsg, portMAX_DELAY) ==
            pdTRUE) {
            LOG(Serial.println("Processing queue message"));

            EspNowChunk currentChunk;
            memcpy(&currentChunk, receivedMsg.payload, receivedMsg.length);

            HAMessage* completeMsg = instance->_handleChunk(currentChunk);

            if (completeMsg != nullptr) {
                LOG(Serial.println("Received complete message"));
                LOG(Serial.printf("Message type: %d",
                                  (uint8_t)completeMsg->messageType));

                delete completeMsg;
            }
        }
    }
}

/**
 * Given a chunk, handle adding it to the correct buffer.
 * @param chunk The chunk to handle.
 * @return A pointer to the complete HAMessage, or a nullptr if
 * the message is not complete yet.
 */
HAMessage* ReceiverTask::_handleChunk(EspNowChunk& chunk) {
    ActiveBuffer* activeBuffer = this->_findActiveBuffer(chunk.chipId);

    // If the buffer is nullptr, initialize it, this is a new message
    if (activeBuffer->buffer == nullptr) {
        LOG(Serial.println("New message received, initializing buffer."));
        activeBuffer->buffer = new uint8_t[chunk.totalLen];
        activeBuffer->chipId = chunk.chipId;
        activeBuffer->readStart = millis();
        activeBuffer->chunksRead = 0;
    }

    if (activeBuffer->chunksRead > chunk.totalChunks) {
        LOG(Serial.println("Received too many chunks. Clearing the buffer."));
        this->_resetActiveBuffer(activeBuffer);
        return nullptr;
    } else if (millis() - activeBuffer->readStart > PAYLOAD_TIMEOUT) {
        LOG(Serial.printf("Payload took longer than %dms to come in, aborting.",
                          PAYLOAD_TIMEOUT));
        this->_resetActiveBuffer(activeBuffer);
        return nullptr;
    }

    // Copy memory from the chunk to the correct location in the active buffer
    memcpy(activeBuffer->buffer + chunk.chunkIndex, &chunk.data, chunk.len);
    activeBuffer->chunksRead += 1;
    activeBuffer->bytesRead += chunk.len;

    if (activeBuffer->chunksRead == chunk.totalChunks) {
        LOG(Serial.println("Finished reading payload"));
        LOG(Serial.println("Validating checksum..."));

        // Validate the checksum
        uint8_t checksum = chunk.checksum;
        for (size_t i = 0; i < activeBuffer->bytesRead; i++) {
            checksum ^= activeBuffer->buffer[i];
        }

        // XOR checksum should be zero if all bytes were received
        if (checksum == 0) {
            LOG(Serial.println("Checksum validation succeeded!"));
            HAMessage* msg = (HAMessage*)activeBuffer->buffer;

            // Reset buffer without freeing memory on the heap so we can return
            // it to the caller.
            this->_resetActiveBuffer(activeBuffer, false);

            return msg;
        } else {
            LOG(Serial.println("Checksum validation failed! Aborting!"));
        }

        this->_resetActiveBuffer(activeBuffer);
    }

    return nullptr;
}

/**
 * Given a chunk ID, finds the current active buffer for that chip,
 * or returns an empty buffer.
 * @param chipId The ID of the chip, coming from the chunk received over espnow
 * @return A pointer to the active buffer or an available buffer.
 */
ActiveBuffer* ReceiverTask::_findActiveBuffer(uint64_t chipId) {
    ActiveBuffer* firstAvailableBuffer = nullptr;

    for (size_t i = 0; i < sizeof(this->_activeBuffers) / sizeof(ActiveBuffer);
         i++) {
        if (this->_activeBuffers[i].chipId == chipId) {
            return &this->_activeBuffers[i];
        } else if (firstAvailableBuffer == nullptr &&
                   this->_activeBuffers[i].chipId == 0) {
            firstAvailableBuffer = &this->_activeBuffers[i];
        }
    }

    return firstAvailableBuffer;
}

/**
 * Given a pointer to one of the buffers, frees the buffer memory
 * and reset the tracking variables.
 * @param buffer The buffer to clear and reset
 * @param free Whether to free the memory on the heap or not. Defaults to true.
 */
void ReceiverTask::_resetActiveBuffer(ActiveBuffer* buffer, bool free) {
    if (free) {
        delete[] buffer->buffer;
    }
    buffer->buffer = nullptr;
    buffer->chunksRead = 0;
    buffer->chipId = 0;
    buffer->readStart = 0;
    buffer->bytesRead = 0;
}

/**
 * Pushes an  espnow message to the queue to be processed by this background
 * task
 * @param msg The message to push to the queue.
 */
void ReceiverTask::pushMsg(esp_now_message_t msg) {
    if (xQueueSend(this->_procQueue, &msg, 0) != pdTRUE) {
        LOG(Serial.println("Queue is full! Unable to process message"));
    }
}
