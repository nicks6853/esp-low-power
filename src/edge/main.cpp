#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
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
uint8_t sendCount = 0;

uint8_t destinationMac[] = {0xD4, 0x8A, 0xFC, 0xA8, 0xA3, 0x24};

/**
 * @brief read temperature from the connected BME280
 * @return The number of chunks sent by the chunker.
 */
uint8_t sendTemperature() {
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

    uint8_t chunksSent =
        chunker.send(destinationMac, (uint8_t*)msg, sizeof(HAMessage));
    delete msg;

    return chunksSent;
}

uint8_t sendHumidity() {
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

    uint8_t chunksSent =
        chunker.send(destinationMac, (uint8_t*)msg, sizeof(HAMessage));
    delete msg;

    return chunksSent;
}

uint8_t createDevice() {
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

    uint8_t chunksSent =
        chunker.send(destinationMac, (uint8_t*)msg, sizeof(*msg));

    delete device;
    delete origin;
    delete[] components;

    return chunksSent;
}

void sendCallback(uint8_t* mac __attribute__((unused)), uint8_t sendStatus) {
    LOG(Serial.printf("Last message sent status: %d\n", sendStatus));
    sendCount++;
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

/**
 * Initializes the BME280 sensor for
 * humidity and temperature readings
 */
void initializeBme() {
    Wire.begin();
    while (!bme.begin()) {
        LOG(Serial.println("Could not find BME280 sensor!"));
        delay(1000);
    }
}

void setup() {
#if defined(DEBUG) && DEBUG == 1
    Serial.begin(ESP_BAUD_RATE);
    while (!Serial);
    LOG(Serial.println("Serial ready!"));
#endif

    uint8_t totalChunks = 0;

    // Set the random seed
    randomSeed(analogRead(A0) + micros());
    initializeBme();
    initializeEspNow();

    rst_info* resetInfo = ESP.getResetInfoPtr();

    // If this is a deep-sleep wake cycle
    if (resetInfo->reason == REASON_DEEP_SLEEP_AWAKE) {
        // Send temperature and humidity over ESPNOW
        totalChunks += sendTemperature();
        totalChunks += sendHumidity();
    } else {
        // This is first power-up, send discovery message
        totalChunks += createDevice();
    }

    // Wait for all chunks to have been sent by ESPNOW
    while (sendCount < totalChunks) {
        delay(1);
    }

    // Go to sleep for 5 seconds. Zzz...
    LOG(Serial.println("Going to sleep..."));
    ESP.deepSleep(5e6);
}

void loop() {}
