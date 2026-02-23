#include <Arduino.h>
#include <ArduinoJson.h>
#include <BME280I2C.h>
#include <Wire.h>

#include "HardwareSerial.h"
#include "config.h"
#include "config_template.h"
#include "ha_manager.h"
#include "mqtt_client.h"
#include "serial_reader.h"
#include "wifi_manager.h"
#define READ_INTERVAL 60000

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MqttClient mqttClient(MQTT_BROKER, MQTT_USER, MQTT_PASSWORD, MQTT_PORT);
HAManager homeAssistant(mqttClient);
SerialReader* serialReader = new SerialReader(Serial2);
unsigned long lastAction = 0;
unsigned long currentTime;

/**
 * Checks the serial connection to see if any result
 * came from it. If a message came in, publishes it to home
 * assistant.
 */
void processSerial() {
    HAMessage* result = serialReader->read();

    if (result != nullptr) {
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
    }
}

void setup() {
#if defined(DEBUG) && DEBUG == 1
    Serial.begin(ESP_BAUD_RATE);
    while (!Serial);
#endif
    LOG(Serial.println("Serial ready!"));

    // Start serial connection with CTS/RTS pins
    Serial2.setPins(RX2, TX2, ROUTER_CTS, ROUTER_RTS);
    Serial2.setHwFlowCtrlMode(UART_HW_FLOWCTRL_CTS_RTS);
    Serial2.begin(ESP_BAUD_RATE);
    while (!Serial2);

    wifiManager.connect();
    mqttClient.connect();
}

void loop() {
    currentTime = millis();

    if (currentTime - lastAction >= 1000) {
        Serial.println(
            "===============================================================");
        Serial.printf("HeapMemory: %d\n", ESP.getHeapSize());
        Serial.println(
            "===============================================================");
        lastAction = currentTime;
    }

    mqttClient.check_connection();
    wifiManager.check_connection();

    processSerial();
}
