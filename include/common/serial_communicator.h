#ifndef SERIALCOMMUNICATOR_H
#define SERIALCOMMUNICATOR_H

#include <Arduino.h>

#include "message_type.h"

enum SerialCommunicatorState { IDLE, READING };

class SerialCommunicator {
   private:
    HardwareSerial& _serial;
    SerialCommunicatorState _currentState = IDLE;
    size_t _readIndex = 0;
    uint8_t* _readBuffer = nullptr;
    size_t _readSize = 0;
    MessageType _readMessageType;

    void _flushSerial();
    void _reset();
    void _handleIdle();
    void _handleWaitForType();
    HAMessage* _handleReading();

   public:
    SerialCommunicator(HardwareSerial& serial) : _serial(serial) {};

    HAMessage* read();
};
#endif
