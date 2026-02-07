/**
 * Notes:
 * The receiver should:
 * - Wait for messages to come in on ESPNOW
 * - Add those messages to a queue xQueueSendToBack(queueName, incomingData, 0)
 * - Have the main loop process things from the queue, 1 by 1 (with
 * xQueueReceiver(sensorQueue, &pendingData, portMAX_DELAY)
 * - Main loop needs to wait for the "clear to send" line to be LOW from the
 * router.
 *
 *
 * Look up Serial.setPins() and Serial.setHwFlowCtrlMode(UART_HW_FLOWCTRL_CTS)
 * and  Serial.setHwFlowCtrlMode(UART_HW_FLOWCTRL_RTS) for a method to automate
 * the RTS / CTS flow control. ONLY AVAILABLE FOR ESP32, and would be REQUIRED
 * ON BOTH RECEIVER AND ROUTER, so maybe not viable, but you can implement the
 * concept manually pretty easily.
 */
#include <Arduino.h>
#include <config.h>
#include <constants.h>
#include <message_type.h>

#include "HardwareSerial.h"
#include "config.h"

enum SerialState { WAIT = 0, WAIT_FOR_TYPE = 1, READING = 2 };

SerialState state = SerialState::WAIT;
size_t readIndex = 0;  // Max indexes a 256 array
MessageType messageType;
uint8_t* readBuffer = nullptr;
size_t messageSize;

void resetReaderState() {
    // Reset the buffer
    delete[] readBuffer;
    readBuffer = nullptr;
    readIndex = 0;
    state = SerialState::WAIT;
}

void flushSerial() {
    Serial.println("Flushing serial buffer...");
    while (Serial2.available() > 0) {
        Serial2.read();
    }
    Serial.println("Buffer flushed");
}

void setup() {
    Serial2.begin(ESP_BAUD_RATE, SERIAL_8N1, RX2, TX2);
    Serial2.setTimeout(100);  // Set timeout to 100ms to prevent hanging too
                              // long if readBytes gets stuck
    Serial.begin(ESP_BAUD_RATE);

    while (!Serial || !Serial2);
    Serial.println("Serial ready!");
    delay(500);
}

void loop() {
    if (Serial2.available() > 0) {
        switch (state) {
            case SerialState::WAIT: {
                uint8_t incomingByte = Serial2.read();
                if (incomingByte == MESSAGE_START) {
                    Serial.println("Started reading");

                    state = SerialState::WAIT_FOR_TYPE;
                    readIndex = 0;
                }
                break;
            }
            case SerialState::WAIT_FOR_TYPE: {
                uint8_t incomingByte = Serial2.read();
                messageType = (MessageType)incomingByte;
                Serial.printf("Message type: %d\n", (uint8_t)messageType);
                switch (messageType) {
                    case MessageType::DEVICE: {
                        messageSize = sizeof(HADevice);
                        readBuffer = new uint8_t[sizeof(HADevice)];
                        state = SerialState::READING;
                        break;
                    }
                    default: {
                        Serial.println("Not handled");
                        state = SerialState::WAIT;
                        break;
                    }
                }
                break;
            }
            case SerialState::READING: {
                // Save to our buffer
                size_t amountRead = Serial2.readBytes(readBuffer, messageSize);

                if (amountRead == messageSize) {
                    if (Serial2.read() == MESSAGE_END) {
                        Serial.printf(
                            "Finished reading! Total bytes: %d, Should be: "
                            "%d\n",
                            readIndex, sizeof(HADevice));

                        HADevice* device = (HADevice*)readBuffer;
                        Serial.printf(
                            "name: %s\nids: %s\nmdl: %s\nmf: %s\ntype: %d\n",
                            device->name, device->ids, device->mdl, device->mf,
                            (uint8_t)device->type);
                    } else {
                        Serial.println("End marker missing (shifted data)");
                        break;
                    }
                } else {
                    Serial.println("Timeout reached before struct was full.");
                    flushSerial();
                    Serial.println("Waiting for next start marker...");
                }

                resetReaderState();
            }
        }
    }
}
