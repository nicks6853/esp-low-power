#include <Arduino.h>
#include <Wire.h>

#include "BME280I2C.h"
#include "components_builder.h"
#include "config.h"
#include "constants.h"
#include "device_builder.h"
#include "message_type.h"
#include "origin_builder.h"
#include "serializable.h"
#include "state_update_builder.h"

bool canSend = true;
BME280I2C bme;
unsigned long lastReading = 0;
unsigned long currentTime;

/**
 * @brief read temperature from the connected BME280
 */
void readTemperature() {
    float temperature = bme.temp();
    Serial.printf("Temperature read: %.2f°C\n", temperature);

    HAStateUpdate<float> state;

    const char* stateTopic = "multisensor/temp/state";
    strncpy(state.topic, stateTopic, sizeof(state.topic));
    state.topic[sizeof(state.topic) - 1] = '\0';

    state.value = temperature;

    // homeAssistant.publishState(state);
}

void writeToSerial() {
    Serial.println("Writing to serial...");

    StateUpdateBuilder<float> stateUpdateBuilder;

    HAStateUpdate<float>* stateUpdate =
        stateUpdateBuilder.withTopic("multisensor/temp/state")
            .withValue(bme.temp())
            .build();

    HAMessage msg(MessageType::STATE_UPDATE_FLOAT);
    memcpy(&msg.stateUpdateF, stateUpdate, sizeof(HAStateUpdate<float>));

    Serial2.write(MESSAGE_START);
    Serial2.write((uint8_t)msg.messageType);
    Serial2.write((uint8_t*)&msg, sizeof(msg));
    Serial2.write(MESSAGE_END);
    Serial.println("Wrote to serial");
    delete stateUpdate;
}

void sendDiscovery() {
    Serial.println("Publishing autodiscovery message");

    // =========== Initialize Device =========== //
    DeviceBuilder deviceBuilder;
    HADevice* device = deviceBuilder.withIds("multisensor_1")
                           .withManufacturer("Nicolas Saulnier")
                           .withModel("TemperatureHumidity")
                           .withName("MultiSensor")
                           .build();

    // =========== Initialize Origin =========== //
    OriginBuilder originBuilder;
    HAOrigin* origin = originBuilder.withName("Saulnier Software Solutions")
                           .withSoftwareVersion("0.0.1-rcbeta")
                           .withUrl("https://www.nsaulnier.com")
                           .build();

    // =========== Initialize Components =========== //
    ComponentsBuilder componentsBuilder;
    HAComponent* components = componentsBuilder.addComponent("temp1")
                                  .withDeviceClass("temperature")
                                  .withPlatform("sensor")
                                  .withStateTopic("multisensor/temp/state")
                                  .withUniqueId("multisensor_temp_1")
                                  .withUnitOfMeasurement("°C")
                                  .withName("Temperature Reading")
                                  .completeComponent()
                                  .addComponent("hum1")
                                  .withDeviceClass("humidity")
                                  .withPlatform("sensor")
                                  .withStateTopic("multisensor/hum/state")
                                  .withUniqueId("multisensor_hum_1")
                                  .withUnitOfMeasurement("%")
                                  .withName("Humidity Reading")
                                  .completeComponent()
                                  .build();

    HADiscoveryPayload discoveryPayload;
    memcpy(&discoveryPayload.dev, device, sizeof(*device));
    memcpy(&discoveryPayload.origin, origin, sizeof(*origin));
    memcpy(&discoveryPayload.cmps, components, sizeof(*components));
    discoveryPayload.cmpCount = 2;

    HAMessage msg(MessageType::DISCOVERY_PAYLOAD);
    msg.discovery = discoveryPayload;
    Serial2.write(MESSAGE_START);
    Serial2.write((uint8_t)msg.messageType);
    Serial2.write((uint8_t*)&msg, sizeof(msg));
    Serial2.write(MESSAGE_END);

    delete device;
    delete origin;
    delete[] components;
}

void setup() {
    /**
     * Initialize serial and temp / hum sensor
     */
    Serial.begin(ESP_BAUD_RATE);
    Serial2.begin(ESP_BAUD_RATE, SERIAL_8N1, RX2,
                  TX2);  // Serial connection to the other ESP
    Wire.begin();

    while (!Serial || !Serial2);
    while (!bme.begin()) {
        Serial.println("Could not find BME280 sensor!");
        delay(1000);
    }

    Serial.println(
        "================================================================"
        "\n\n\n\n\n");

    sendDiscovery();
}

void loop() {
    if (Serial2.available() > 0) {
        uint8_t incoming = Serial2.read();

        if (incoming == XOFF) canSend = false;  // Received XOFF
        if (incoming == XON) canSend = true;    // Received XON
    }

    currentTime = millis();

    if (currentTime - lastReading >= 5000) {
        if (canSend) {
            Serial.println("Not busy, writing");
            writeToSerial();
            lastReading = currentTime;
        } else {
            Serial.println("Busy....");
        }
    }
}
