#include <Arduino.h>

#include "BME280I2C.h"
#include "components_builder.h"
#include "config.h"
#include "device_builder.h"
#include "esp32-hal.h"
#include "esplp_espnow_manager.h"
#include "message_type.h"
#include "origin_builder.h"

EspNowManager* espNowManager;
HADiscoveryPayload discoveryPayload;
BME280I2C bme;
unsigned long lastReading = 0;

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
    state.messageType = MessageType::STATE_UPDATE_FLOAT;

    // homeAssistant.publishState(state);
}

void readHumidity() {
    float humidity = bme.hum();
    Serial.printf("Humidity read: %.2f%%\n", humidity);

    HAStateUpdate<float> state;

    const char* stateTopic = "multisensor/hum/state";
    strncpy(state.topic, stateTopic, sizeof(state.topic));
    state.topic[sizeof(state.topic) - 1] = '\0';

    state.value = humidity;
    state.messageType = MessageType::STATE_UPDATE_FLOAT;

    // homeAssistant.publishState(state);
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

    HADiscoveryPayload payload;
    payload.dev = device;
    payload.origin = origin;
    payload.cmps = components;
    payload.cmpCount = 2;

    if (homeAssistant.discovery(payload)) {
        Serial.println("Successfully published to discovery");
    } else {
        Serial.println("Failed to publish to discovery");
    }

    delete device;
    delete origin;
    delete[] components;
}

void callback(uint8_t* mac, uint8_t sendStatus) {
    Serial.printf("Last message sent status: %d", sendStatus);
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);
    while (!Serial);
    Serial.println("Ready!");
    delay(2000);
    espNowManager = EspNowManager::getInstance();

    espNowManager->init();
    espNowManager->registerSendCallback(&callback);
}

void loop() {
    // espNowManager->autopair();

    unsigned long currentTime = millis();

    if (currentTime - lastReading >= 60000) {
        float temperature = bme.temp();
        float humidity = bme.hum();

        Serial.printf("Sensor read. Temperature: %.2f, Humidity: %.2f",
                      temperature, humidity);

        lastReading = currentTime;
    }
}
