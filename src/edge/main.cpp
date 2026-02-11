#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#include "BME280I2C.h"
#include "chunk.h"
#include "components_builder.h"
#include "config.h"
#include "device_builder.h"
#include "message_type.h"
#include "origin_builder.h"

HADiscoveryPayload discoveryPayload;
BME280I2C bme;
unsigned long lastReading = 0;

uint8_t destinationMac[] = {0xD4, 0x8A, 0xFC, 0xA8, 0xA3, 0x24};

/**
 * @brief read temperature from the connected BME280
 */
// void readTemperature() {
//     float temperature = bme.temp();
//     Serial.printf("Temperature read: %.2f°C\n", temperature);
//
//     HAStateUpdate<float> state;
//
//     const char* stateTopic = "multisensor/temp/state";
//     strncpy(state.topic, stateTopic, sizeof(state.topic));
//     state.topic[sizeof(state.topic) - 1] = '\0';
//
//     state.value = temperature;
//     state.messageType = MessageType::STATE_UPDATE_FLOAT;
//
//     // homeAssistant.publishState(state);
// }

// void readHumidity() {
//     float humidity = bme.hum();
//     Serial.printf("Humidity read: %.2f%%\n", humidity);
//
//     HAStateUpdate<float> state;
//
//     const char* stateTopic = "multisensor/hum/state";
//     strncpy(state.topic, stateTopic, sizeof(state.topic));
//     state.topic[sizeof(state.topic) - 1] = '\0';
//
//     state.value = humidity;
//     state.messageType = MessageType::STATE_UPDATE_FLOAT;
//
//     // homeAssistant.publishState(state);
// }
//

void createDevice() {
    Serial.println("Adding device");

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

    HAStateUpdate<float> stateUpdate;
    strncpy(stateUpdate.topic, "multisensor/temp/state",
            sizeof(stateUpdate.topic));
    stateUpdate.topic[sizeof(stateUpdate.topic) - 1] = '\0';
    stateUpdate.value = 12.0;

    HAMessage msg(MessageType::STATE_UPDATE_FLOAT);
    memcpy(&msg.stateUpdateF, &stateUpdate, sizeof(stateUpdate));

    delete device;
    delete origin;
    delete[] components;

    // Send over ESPNOW
    EspNowChunker chunker;
    chunker.send(destinationMac, (uint8_t*)&msg, sizeof(msg));
}

void sendCallback(uint8_t* mac, uint8_t sendStatus) {
    Serial.printf("Last message sent status: %d", sendStatus);
}

uint8_t initializeEspNow() {
    Serial.println("Initializing ESPNOW...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Print mac address
    Serial.print("Mac address: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESPNOW!");
        return 0;
    }

    Serial.println("ESPNOW Initialized!");
    return 1;
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);
    while (!Serial);
    Serial.println("Serial ready!");

    initializeEspNow();

    // Set the current role
    uint8_t res = esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    Serial.printf("RESULT FROM SET ROLE: %d", res);
    // Register peer
    res = esp_now_add_peer(destinationMac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    Serial.printf("RESULT FROM ADD PEER: %d", res);

    // Register callback on send
    res = esp_now_register_send_cb(sendCallback);
    Serial.printf("Result from register send callback: %d", res);
}

void loop() {
    // espNowManager->autopair();

    unsigned long currentTime = millis();

    if (currentTime - lastReading >= 2000) {
        // Send a message over ESPNOW
        createDevice();

        // float temperature = bme.temp();
        // float humidity = bme.hum();

        // Serial.printf("Sensor read. Temperature: %.2f, Humidity: %.2f",
        //               temperature, humidity);

        lastReading = currentTime;
    }
}
