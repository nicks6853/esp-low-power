#include <Arduino.h>
#include <ArduinoJson.h>
#include <BME280I2C.h>
#include <Wire.h>

#include "HardwareSerial.h"
#include "config.h"
#include "config_template.h"
#include "ha_manager.h"
#include "mqtt_client.h"
#include "serial_communicator.h"
#include "wifi_manager.h"
#define READ_INTERVAL 60000

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MqttClient mqttClient(MQTT_BROKER, MQTT_USER, MQTT_PASSWORD, MQTT_PORT);
HAManager homeAssistant(mqttClient);
SerialCommunicator serialCommunicator(Serial2);
unsigned long lastPrint = 0;

void setup() {
    Serial.begin(ESP_BAUD_RATE);
    Serial2.begin(ESP_BAUD_RATE, SERIAL_8N1, RX2,
                  TX2);  // Serial connection to the other ESP

    while (!Serial || !Serial2);
    Serial.println("Serial ready!");

    wifiManager.connect();
    mqttClient.connect();

    Serial2.write(XON);
}

void loop() {
    unsigned long currentTime = millis();

    if (currentTime - lastPrint >= 1000) {
        Serial.println(
            "===============================================================");
        Serial.printf("HeapMemory: %d\n", ESP.getHeapSize());
        Serial.println(
            "===============================================================");
        lastPrint = currentTime;
    }

    mqttClient.check_connection();
    wifiManager.check_connection();

    HAMessage* result = serialCommunicator.read();

    if (result != nullptr) {
        // Send XOFF signal
        Serial2.write(XOFF);
        Serial.println("Forwarding result to Home Assistant");
        switch (result->messageType) {
            case MessageType::DISCOVERY_PAYLOAD: {
                Serial.printf("%s", result->payload.discovery.dev.mdl);
                Serial.printf("%s", result->payload.discovery.origin.name);
                Serial.printf("%d", result->payload.discovery.cmpCount);
                Serial.printf("%s", result->payload.discovery.cmps[0].key);
                Serial.printf("%s",
                              result->payload.discovery.cmps[0].value.name);
                Serial.printf("%s",
                              result->payload.discovery.cmps[0].value.stat_t);
                if (homeAssistant.discovery(result->payload.discovery)) {
                    Serial.println("Published discovery successfully");
                } else {
                    Serial.println("Failed to publish discovery");
                }
                break;
            }
            case MessageType::STATE_UPDATE_FLOAT: {
                if (homeAssistant.publishStateUpdate(
                        result->payload.stateUpdateF)) {
                    Serial.println("Published state update successfully");
                } else {
                    Serial.printf("Failed to publish state update of type %d\n",
                                  (uint8_t)result->messageType);
                }
                break;
            }
            default: {
                Serial.printf("Unhandle message type %d",
                              (uint8_t)result->messageType);
                break;
            }
        }

        // Clean up data on heap
        delete result;

        // Send XON signal to say we are ready to read more.
        Serial2.write(XON);
    }
}
