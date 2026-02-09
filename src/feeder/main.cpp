#include <Arduino.h>

#include "config.h"
#include "constants.h"
#include "message_type.h"
#include "state_update_builder.h"

bool canSend = true;

void writeToSerial() {
    Serial.println("Writing to serial...");

    StateUpdateBuilder<float> stateUpdateBuilder;

    HAStateUpdate<float>* stateUpdate =
        stateUpdateBuilder.withTopic("multisensor/temperature/state")
            .withValue(23)
            .build();

    /**
     * TODO: Instead of writing the messsage like that. Build
     * a struct that has a MessageType attribute and a union
     * with all the data types it could send. Then, just send
     * that struct. On the other side of the serial connection,
     * you can just read the MessageType attribute and know what you got.
     */
    Serial2.write(MESSAGE_START);
    Serial2.write((uint8_t)stateUpdate->messageType);
    Serial2.write((uint8_t*)stateUpdate, sizeof(*stateUpdate));
    Serial2.write(MESSAGE_END);
    Serial.println("Wrote to serial");
    delete stateUpdate;
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);
    Serial2.begin(ESP_BAUD_RATE, SERIAL_8N1, RX2,
                  TX2);  // Serial connection to the other ESP

    while (!Serial || !Serial2);
    Serial.println(
        "================================================================"
        "\n\n\n\n\n");
}

void loop() {
    if (Serial2.available() > 0) {
        uint8_t incoming = Serial2.read();

        if (incoming == XOFF) canSend = false;  // Received XOFF
        if (incoming == XON) canSend = true;    // Received XON
    }

    if (canSend) {
        Serial.println("Not busy, writing");
        writeToSerial();
        delay(100);
    } else {
        Serial.println("Busy....");
    }
}
