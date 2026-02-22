#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#include "BME280I2C.h"
#include "chunker.h"
#include "components_builder.h"
#include "config.h"
#include "device_builder.h"
#include "message_type.h"
#include "origin_builder.h"

BME280I2C bme;
unsigned long lastReading = 0;
EspNowChunker chunker;

uint8_t destinationMac[] = {0xD4, 0x8A, 0xFC, 0xA8, 0xA3, 0x24};

/**
 * @brief read temperature from the connected BME280
 */
void sendTemperature() {
    float temperature = bme.temp();
    Serial.printf("Temperature read: %.2f°C\n", temperature);

    HAStateUpdate<float> state;

    const char* stateTopic = "multisensor/temp/state";
    strncpy(state.topic, stateTopic, sizeof(state.topic));
    state.topic[sizeof(state.topic) - 1] = '\0';

    state.value = temperature;

    HAMessage* msg = new HAMessage;
    msg->messageType = MessageType::STATE_UPDATE_FLOAT;

    msg->payload.stateUpdateF = state;

    chunker.send(destinationMac, (uint8_t*)msg, sizeof(HAMessage));
}

void sendHumidity() {
    float humidity = bme.hum();
    Serial.printf("Humidity read: %.2f%%\n", humidity);

    HAStateUpdate<float> state;

    const char* stateTopic = "multisensor/hum/state";
    strncpy(state.topic, stateTopic, sizeof(state.topic));
    state.topic[sizeof(state.topic) - 1] = '\0';

    state.value = humidity;

    HAMessage* msg = new HAMessage;
    msg->messageType = MessageType::STATE_UPDATE_FLOAT;

    msg->payload.stateUpdateF = state;

    chunker.send(destinationMac, (uint8_t*)msg, sizeof(HAMessage));
}

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

    HADiscoveryPayload discoveryPayload;

    discoveryPayload.cmpCount = 2;
    memcpy(&discoveryPayload.dev, device, sizeof(HADevice));
    memcpy(&discoveryPayload.origin, origin, sizeof(HAOrigin));
    memcpy(&discoveryPayload.cmps, components, sizeof(HAComponent) * 2);

    HAMessage* msg = new HAMessage;
    msg->messageType = MessageType::DISCOVERY_PAYLOAD;
    msg->payload.discovery = discoveryPayload;

    chunker.send(destinationMac, (uint8_t*)msg, sizeof(*msg));

    delete device;
    delete origin;
    delete[] components;
}

void sendCallback(uint8_t* mac __attribute__((unused)), uint8_t sendStatus) {
    LOG(Serial.printf("Last message sent status: %d\n", sendStatus));
}

uint8_t initializeEspNow() {
    LOG(Serial.println("Initializing ESPNOW..."));

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Print mac address
    LOG(Serial.print("Mac address: "));
    LOG(Serial.println(WiFi.macAddress()));

    if (esp_now_init() != 0) {
        LOG(Serial.println("Error initializing ESPNOW!"));
        return 0;
    }

    // Set the current role
    uint8_t res = esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    LOG(Serial.printf("RESULT FROM SET ROLE: %d", res));
    // Register peer
    res = esp_now_add_peer(destinationMac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    LOG(Serial.printf("RESULT FROM ADD PEER: %d", res));

    // Register callback on send
    res = esp_now_register_send_cb(sendCallback);
    LOG(Serial.printf("Result from register send callback: %d", res));

    LOG(Serial.println("ESPNOW Initialized!"));
    return 1;
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);
    while (!Serial);
    Serial.println("Serial ready!");

    initializeEspNow();

    // Send the discovery message for Home Assistant
    createDevice();
}

void loop() {
    // espNowManager->autopair();

    unsigned long currentTime = millis();

    if (currentTime - lastReading >= 5000) {
        // Send temperature and humidity over ESPNOW
        sendTemperature();
        sendHumidity();
        lastReading = currentTime;
    }
}
