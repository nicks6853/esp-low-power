#ifndef SERIALCOMMUNICATOR_H
#define SERIALCOMMUNICATOR_H

#include <Arduino.h>

#include "message_type.h"

enum SerialCommunicatorState { SERIAL_WAIT, SERIAL_READING };

class SerialCommunicator {
   private:
    HardwareSerial* _serial;
    SerialCommunicatorState _currentState = SERIAL_WAIT;
    size_t _readIndex = 0;
    uint8_t _readBuffer[sizeof(HADiscoveryPayload)];

   public:
    SerialCommunicator(HardwareSerial* serial) : _serial(serial) {};

    void write(const HADiscoveryPayload* body);
    uint8_t read(HADiscoveryPayload* buffer);
};
#endif
