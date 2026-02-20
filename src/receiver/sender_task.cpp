#include "sender_task.h"

#include "message_type.h"

SenderTask::SenderTask(const char* name, uint16_t stackSize,
                       UBaseType_t priority) {
    // Initialize the queue
    this->_procQueue = xQueueCreate(20, sizeof(HAMessage));

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
void SenderTask::taskBody(void* pvParameters) {
    SenderTask* instance = (SenderTask*)pvParameters;
    HAMessage* receivedMsg = new HAMessage;

    while (true) {
        if (xQueueReceive(instance->_procQueue, receivedMsg, portMAX_DELAY) ==
            pdTRUE) {
            LOG(Serial.println("SENDER - Processing queue message"));
            LOG(Serial.printf("Message RECEIVED TYPE: %d\n",
                              (uint8_t)receivedMsg->messageType));

            // Cleanup
            delete receivedMsg;
            receivedMsg = nullptr;
        }
    }
}

/**
 * Given a message, handle sending it to over the Serial connection
 * @param msg The message to handle.
 */
void SenderTask::_handleMessage(HAMessage* msg) {}

/**
 * Pushes an  espnow message to the queue to be processed by this background
 * task
 * @param msg The message to push to the queue.
 */
void SenderTask::pushMsg(const HAMessage* msg) {
    if (xQueueSend(this->_procQueue, msg, 0) != pdTRUE) {
        LOG(Serial.println("Queue is full! Unable to process message"));
    }
}
