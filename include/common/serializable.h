#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H
#include <Arduino.h>
struct Serializable {
    /**
     * Write data to the provided HardwareSerial.
     * Inheriting classes to implement details.
     */
    virtual uint8_t write(HardwareSerial& serial) = 0;
};
#endif
