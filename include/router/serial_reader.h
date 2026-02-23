#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <Arduino.h>

#include "message_type.h"

enum SerialReaderState { IDLE, READING };

class SerialReader {
   private:
    HardwareSerial& _serial;
    SerialReaderState _currentState = IDLE;
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
    SerialReader(HardwareSerial& serial) : _serial(serial) {};

    HAMessage* read();
};
#endif
