#include <Arduino.h>
#include <ArduinoJson.h>
#include <BME280I2C.h>
#include <Wire.h>

#include "HardwareSerial.h"
#include "components_builder.h"
#include "config.h"
#include "device_builder.h"
#include "ha_manager.h"
#include "message_type.h"
#include "mqtt_client.h"
#include "origin_builder.h"
#include "serial_communicator.h"
#include "wifi_manager.h"
#define CAPACITIVE_TOUCH_PIN 4
#define READ_INTERVAL 60000

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MqttClient mqttClient(MQTT_BROKER, MQTT_USER, MQTT_PASSWORD, MQTT_PORT);
HardwareSerial SerialPort(1);
SerialCommunicator serialCommunicator(&SerialPort);
HADiscoveryPayload discoveryPayload;
HAManager homeAssistant(mqttClient);
BME280I2C bme;

unsigned long touchStart = 0;
bool touched = 0;
touch_value_t touchValue;

unsigned long lastReading = -READ_INTERVAL;

void readTemperature() {
    float temperature = bme.temp();
    Serial.printf("Temperature read: %.2f°C\n", temperature);

    HAStateUpdate<float> state;

    const char* stateTopic = "multisensor/temp/state";
    strncpy(state.topic, stateTopic, sizeof(state.topic));
    state.topic[sizeof(state.topic) - 1] = '\0';

    state.value = temperature;
    state.valueType = ValueType::FLOAT_TYPE;

    homeAssistant.publishState(state);
}

void readHumidity() {
    float humidity = bme.hum();
    Serial.printf("Humidity read: %.2f%%\n", humidity);

    HAStateUpdate<float> state;

    const char* stateTopic = "multisensor/hum/state";
    strncpy(state.topic, stateTopic, sizeof(state.topic));
    state.topic[sizeof(state.topic) - 1] = '\0';

    state.value = humidity;
    state.valueType = ValueType::FLOAT_TYPE;

    homeAssistant.publishState(state);
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

void checkTouch() {
    touchValue = touchRead(CAPACITIVE_TOUCH_PIN);

    // Check how long capacitive pin GPIO4 is touched and do
    // different actions
    if (!touched && touchValue <= 50) {
        Serial.println("Touch start");
        touched = 1;
        touchStart = millis();
    } else if (touched && touchValue > 50) {
        Serial.println("\nTouch ended");
        unsigned long totalTouchTime = millis() - touchStart;

        if (totalTouchTime >= 2000) {
            readTemperature();
        } else if (totalTouchTime >= 1000) {
            createDevice();
        }

        // Reset touch tracking
        touched = 0;
        touchStart = 0;
    } else if (touched && touchValue <= 50) {
        Serial.printf("\rTouched for: %ldms", millis() - touchStart);
    }
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);
    Wire.begin();
    // SerialPort.begin(ESP_BAUD_RATE, SERIAL_8N1, RX1, TX1);

    // while (!Serial || !SerialPort);
    while (!Serial);

    while (!bme.begin()) {
        Serial.println("Could not find BME280 sensor!");
        delay(1000);
    }

    Serial.println("Starting program");

    wifiManager.connect();
    mqttClient.connect();
    Serial.println("Waiting for input.");
}

void loop() {
    mqttClient.check_connection();
    wifiManager.check_connection();
    checkTouch();

    if (millis() - lastReading >= READ_INTERVAL) {
        readTemperature();
        readHumidity();
        lastReading = millis();
    }

    // mqttClient.publish("testing/something", "payload");
    // uint8_t result = serialCommunicator.read(&discoveryPayload);
    // if (result) {
    //     Serial.printf("Message received: name: %s, url: %s\n",
    //                   discoveryPayload.origin.name,
    //                   discoveryPayload.origin.url);
    // }
}
