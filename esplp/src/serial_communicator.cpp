#include "serial_communicator.h"

#include <Arduino.h>

#include "constants.h"
#include "message_type.h"

void SerialCommunicator::write(const HADiscoveryPayload* message) {
    this->_serial->write(MESSAGE_START);
    this->_serial->write((uint8_t*)message, sizeof(*message));
    this->_serial->write(MESSAGE_END);
}

uint8_t SerialCommunicator::read(HADiscoveryPayload* buffer) {
    uint8_t byteReceived;

    // Loop while there are bytes available to read
    // on the serial connection
    while (this->_serial->available()) {
        // Read a byte from the serial connection
        byteReceived = this->_serial->read();

        switch (this->_currentState) {
            // Handle the wait state
            case SERIAL_WAIT:
                // Begin reading when we receive a MESSAGE_START
                // byte that indicates the start of a message.
                if (byteReceived == MESSAGE_START) {
                    this->_readIndex = 0;
                    this->_currentState = SERIAL_READING;
                }
                break;
            case SERIAL_READING:
                if (byteReceived == MESSAGE_END) {
                    // Stop reading when we receive a MESSAGE_END
                    // byte that indicates the end of a message.
                    if (this->_readIndex == sizeof(HADiscoveryPayload)) {
                        // Copy the resulting message into the provided buffer
                        memcpy(buffer, this->_readBuffer,
                               sizeof(HADiscoveryPayload));

                        // Reset the serial communicator to be ready to read
                        // another message
                        this->_currentState = SERIAL_WAIT;
                        Serial.println("Finished reading");
                        return 1;
                    } else {
                        // We didn't receive the expected message size,
                        // something is wrong
                        Serial.println(
                            "Too much data received from the serial "
                            "connection!");
                        return 0;
                    }
                } else {
                    if (_readIndex < sizeof(HADiscoveryPayload)) {
                        this->_readBuffer[this->_readIndex++] = byteReceived;
                    } else {
                        return 0;
                    }
                }
                break;
            default:
                // TODO: Log somewhere that the state is unknown / unhandled
                Serial.println("Unable to determine communicator state");
                return 0;
                break;
        }
    }

    return 0;
}
