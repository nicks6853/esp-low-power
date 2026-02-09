#ifndef SERIALCOMMUNICATOR_H
#define SERIALCOMMUNICATOR_H

#include <Arduino.h>

#include "message_type.h"
#include "serializable.h"

enum SerialCommunicatorState { IDLE, WAIT_FOR_TYPE, READING };

struct IncomingMessage {
    MessageType type;
    union {
        HAStateUpdate<float>* floatData;
        HAStateUpdate<int>* intData;
        HAStateUpdate<char[128]>* charData;
        HADiscoveryPayload* discoveryData;
    };
    ~IncomingMessage();
};

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
    IncomingMessage* _handleReading();

   public:
    SerialCommunicator(HardwareSerial& serial) : _serial(serial) {};

    void write(const Serializable& body);

    IncomingMessage* read();
};
#endif
