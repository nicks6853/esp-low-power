#include <Arduino.h>

#include "HardwareSerial.h"
#include "config.h"
#include "constants.h"
#include "device_builder.h"
#include "message_type.h"

void writeToSerial() {
    Serial.println("Writing to serial...");

    DeviceBuilder builder;
    HADevice* device = builder.withIds("multisensor_1")
                           .withManufacturer("Nicolas Saulnier")
                           .withModel("TemperatureHumidity")
                           .withName("MultiSensor")
                           .build();

    Serial1.write(MESSAGE_START);
    Serial1.write((uint8_t)device->type);
    Serial1.write((uint8_t*)device, sizeof(HADevice));
    Serial1.write(MESSAGE_END);
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
