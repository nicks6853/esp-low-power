#include <Arduino.h>

#include "config.h"
#include "device_builder.h"
#include "message_type.h"

void writeToSerial() {
    Serial.println("Writing to serial...");

    DeviceBuilder builder;

    HAStateUpdate<char[128]> stateUpdate;

    const char* topic = "multisensor/temp/state";

    strncpy(stateUpdate.topic, topic, sizeof(stateUpdate.topic));
    stateUpdate.topic[sizeof(stateUpdate.topic) - 1] = '\0';

    strncpy(stateUpdate.value, "SomeValue", sizeof(stateUpdate.value));
    stateUpdate.value[sizeof(stateUpdate.value) - 1] = '\0';

    stateUpdate.write(Serial1);
    Serial.println("Wrote to serial");
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);
    Serial1.begin(ESP_BAUD_RATE);  // Serial to the other device
                                   //
    while (!Serial || !Serial1);
    Serial.println(
        "================================================================"
        "\n\n\n\n\n");
    writeToSerial();
}

void loop() {}
