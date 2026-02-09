#include "serial_communicator.h"

#include <Arduino.h>

#include "config.h"
#include "constants.h"
#include "message_type.h"
#include "serializable.h"

IncomingMessage::~IncomingMessage() {
    switch (this->type) {
        case (MessageType::STATE_UPDATE_FLOAT): {
            delete floatData;
            break;
        }
        case (MessageType::STATE_UPDATE_INT): {
            delete intData;
            break;
        }
        case (MessageType::STATE_UPDATE_CHAR_128): {
            delete charData;
            break;
        }
        case (MessageType::DISCOVERY_PAYLOAD): {
            delete discoveryData;
            break;
        }
        default: {
            Serial.printf(
                "Destructor not set for message type %d. This might cause a "
                "memory "
                "leak",
                (uint8_t)this->type);
            break;
        }
    }
}

void SerialCommunicator::write(const Serializable& body) {
    this->_serial.write(MESSAGE_START);
    body.write(this->_serial);
    this->_serial.write(MESSAGE_END);
}

void SerialCommunicator::_flushSerial() {
    Serial.println("Flushing serial buffer...");
    while (this->_serial.available() > 0) {
        this->_serial.read();
    }
    Serial.println("Buffer flushed");
}

void SerialCommunicator::_reset() {
    /**
     * NOTE: this->_readBuffer is not deleted
     * because it is returned to the caller when this->read()
     * is called.
     * The caller is responsible for the memory on the heap.
     */
    this->_readBuffer = nullptr;
    this->_readIndex = 0;
    this->_readSize = 0;
    this->_currentState = SerialCommunicatorState::IDLE;
    delay(1000);

    // Send XON signal
    this->_serial.write(XON);
}

void SerialCommunicator::_handleIdle() {
    uint8_t incomingByte = this->_serial.read();
    if (incomingByte == MESSAGE_START) {
        Serial.println("Started reading");

        this->_currentState = SerialCommunicatorState::WAIT_FOR_TYPE;
        this->_readIndex = 0;

        // Send XOFF signal
        this->_serial.write(XOFF);
    }
}

/**
 * @brief Handles the "wait for type" state of the serial communicator.
 * Waits for the byte that indicates what type of struct is coming
 * comes in.
 */
void SerialCommunicator::_handleWaitForType() {
    uint8_t incomingByte = this->_serial.read();
    this->_readMessageType = (MessageType)incomingByte;
    Serial.printf("Message type: %d\n", (uint8_t)this->_readMessageType);
    switch (this->_readMessageType) {
        case MessageType::STATE_UPDATE_FLOAT: {
            this->_readSize = sizeof(HAStateUpdate<float>);
            this->_readBuffer = new uint8_t[sizeof(HAStateUpdate<float>)];
            this->_currentState = SerialCommunicatorState::READING;
            break;
        }
        default: {
            Serial.println("Not handled");
            this->_currentState = SerialCommunicatorState::IDLE;
            break;
        }
    }
}

/**
 * @brief Handles reading the struct bytes from the Serial connection.
 * @return A pointer to the complete struct or nullptr if we are not done
 *         reading or there is nothing to read.
 */
IncomingMessage* SerialCommunicator::_handleReading() {
    // Prepare response pointer, stays null if we didn't finish reading.
    IncomingMessage* returnMsg = nullptr;

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
             * types and return the result as an IncomingMessage struct.
             */

            Serial.println("Read complete object");

            switch (this->_readMessageType) {
                case MessageType::STATE_UPDATE_FLOAT: {
                    returnMsg = new IncomingMessage;
                    returnMsg->type = this->_readMessageType;
                    returnMsg->floatData =
                        (HAStateUpdate<float>*)this->_readBuffer;

                    break;
                }
                default: {
                    Serial.println("Unsupported type");
                    break;
                }
            }

        } else {
            Serial.println("End marker missing (shifted data)");
            delete[] this->_readBuffer;
            this->_readBuffer = nullptr;
        }
        this->_reset();
    } else {
        Serial.println("Timeout reached before struct was full.");
        delete[] this->_readBuffer;
        this->_readBuffer = nullptr;
        this->_flushSerial();
        this->_reset();
        Serial.println("Waiting for next start marker...");
    }

    return returnMsg;
}

/**
 * @brief Reads incoming structs on the Serial connection.
 * @return A pointer to the incoming message or nullptr if nothing
 *         has been received.
 */
IncomingMessage* SerialCommunicator::read() {
    if (this->_serial.available() > 0) {
        switch (this->_currentState) {
            case SerialCommunicatorState::IDLE: {
                this->_handleIdle();
                break;
            }
            case SerialCommunicatorState::WAIT_FOR_TYPE: {
                this->_handleWaitForType();
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
