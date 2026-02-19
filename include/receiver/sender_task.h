#ifndef SENDERTASK_H
#define SENDERTASK_H
#include <Arduino.h>

#include "config.h"
#include "message_type.h"

#ifdef DEBUG
#define LOG(x) x
#else
#define LOG(X) (void)0
#endif

class SenderTask {
   private:
    QueueHandle_t _procQueue;
    TaskHandle_t _task;

    void _handleMessage(HAMessage* msg);

   public:
    SenderTask(const char* name, uint16_t stackSize, UBaseType_t priority);
    static void taskBody(void* pvParameters);
    void pushMsg(HAMessage msg);
};
#endif
