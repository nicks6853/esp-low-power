#include "serial_communicator.h"

#include <Arduino.h>

#include "constants.h"
#include "message_type.h"

void SerialCommunicator::_flushSerial() {
    Serial.println("Flushing serial buffer...");
    while (this->_serial.available() > 0) {
        this->_serial.read();
    }
    Serial.println("Buffer flushed");
}

/**
 * Resets the serial communicator. Clears the read buffer,
 * tracking indices and current state.
 */
void SerialCommunicator::_reset() {
    /**
     * Intentionally reset the buffer
     * without deleting the data on the heap
     * because it is returned to the caller
     */
    this->_readBuffer = nullptr;

    this->_readIndex = 0;
    this->_readSize = 0;

    this->_currentState = SerialCommunicatorState::IDLE;
}

/**
 * @brief Handles the idle state, waiting for a message come in.
 */
void SerialCommunicator::_handleIdle() {
    uint8_t incomingByte = this->_serial.read();
    if (incomingByte == MESSAGE_START) {
        Serial.println("Started reading");

        this->_readIndex = 0;
        this->_readSize = sizeof(HAMessage);
        this->_readBuffer = new uint8_t[sizeof(HAMessage)];
        this->_currentState = SerialCommunicatorState::READING;
    }
}

/**
 * @brief Handles reading HAMesages from the Serial connection.
 * @return A 1 or 0 indicating success or failure.
 */
HAMessage* SerialCommunicator::_handleReading() {
    // Save to our buffer
    size_t amountRead =
        this->_serial.readBytes(this->_readBuffer + this->_readIndex,
                                this->_readSize - this->_readIndex);

    // Add to the tally of how many bytes were read
    this->_readIndex += amountRead;

    if (this->_readIndex == this->_readSize) {
        if (this->_serial.read() == MESSAGE_END) {
            /**
             * Here, we've read all the bytes, handle the different
             * types and return the result as an HAMessage struct.
             */

            Serial.println("Read complete object");
            HAMessage* returnPtr = (HAMessage*)this->_readBuffer;

            this->_reset();

            return returnPtr;
        } else {
            Serial.println("End marker missing (shifted data)");
            delete[] this->_readBuffer;
            this->_reset();
        }
    } else {
        Serial.println("Timeout reached before struct was full.");
        this->_flushSerial();
        delete[] this->_readBuffer;
        this->_reset();
        Serial.println("Waiting for next start marker...");
    }
    return nullptr;
}

/**
 * @brief Reads incoming HAMessage structs on the Serial connection.
 * @return Returns a pointer to the item read, or nullptr if nothing was read
 */
HAMessage* SerialCommunicator::read() {
    if (this->_serial.available() > 0) {
        switch (this->_currentState) {
            case SerialCommunicatorState::IDLE: {
                this->_handleIdle();
                break;
            }
            case SerialCommunicatorState::READING: {
                return this->_handleReading();
                break;
            }
        }
    }
    return nullptr;
}
