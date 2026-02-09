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
}

void loop() {
    unsigned long currentTime = millis();

    if (currentTime - lastPrint >= 1000) {
        Serial.printf("HeapMemory: %d\n", ESP.getHeapSize());
        lastPrint = currentTime;
    }

    mqttClient.check_connection();
    wifiManager.check_connection();

    IncomingMessage* result = serialCommunicator.read();

    if (result != nullptr) {
        Serial.println("HAD A RESULT!");
        Serial.printf("Result type: %d\n\ttopic: %s\n\tvalue: %.2f\n",
                      (uint8_t)result->type, result->floatData->topic,
                      result->floatData->value);
        delete result;
    }
}
