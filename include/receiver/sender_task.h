#ifndef SENDERTASK_H
#define SENDERTASK_H
#include <Arduino.h>

#include "message_type.h"

class SenderTask {
   private:
    QueueHandle_t _procQueue;
    TaskHandle_t _task;

    void _handleMessage(const HAMessage* msg);

   public:
    SenderTask(const char* name, uint16_t stackSize, UBaseType_t priority);
    static void taskBody(void* pvParameters);
    void pushMsg(const HAMessage* msg);
};
#endif
