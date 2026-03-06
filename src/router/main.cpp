#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>

#include "HardwareSerial.h"
#include "config.h"
#include "config_template.h"
#include "ha_manager.h"
#include "mqtt_client.h"
#include "oled_terminal.h"
#include "serial_reader.h"
#include "wifi_manager.h"
#define READ_INTERVAL 60000

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MqttClient mqttClient(MQTT_BROKER, MQTT_USER, MQTT_PASSWORD, MQTT_PORT);
HAManager homeAssistant(mqttClient);
OledTerminal* oledTerminal = new OledTerminal(128, 32, 0x3C);
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
                if (homeAssistant.discovery(result->payload.discovery)) {
                    LOG(Serial.println("Published discovery successfully"));
                    oledTerminal->appendLine("Discovery");
                } else {
                    LOG(Serial.println("Failed to publish discovery"));
                    oledTerminal->appendLine("Failed Discovery");
                }
                break;
            }
            case MessageType::STATE_UPDATE_FLOAT: {
                if (homeAssistant.publishStateUpdate(
                        result->payload.stateUpdateF)) {
                    LOG(Serial.println("Published state update successfully"));
                    char msg[32];
                    snprintf(msg, 32, "%s", result->payload.stateUpdateF.topic);
                    oledTerminal->appendLine(msg);
                } else {
                    LOG(Serial.printf(
                        "Failed to publish state update of type %d\n",
                        (uint8_t)result->messageType));
                    char msg[32];
                    snprintf(msg, 32, "!%s",
                             result->payload.stateUpdateF.topic);
                    oledTerminal->appendLine(msg);
                }
                break;
            }
            case MessageType::STATE_UPDATE_INT: {
                if (homeAssistant.publishStateUpdate(
                        result->payload.stateUpdateI)) {
                    LOG(Serial.println("Published state update successfully"));
                    char msg[32];
                    snprintf(msg, 32, "%s", result->payload.stateUpdateI.topic);
                    oledTerminal->appendLine(msg);
                } else {
                    LOG(Serial.printf(
                        "Failed to publish state update of type %d\n",
                        (uint8_t)result->messageType));
                    char msg[32];
                    snprintf(msg, 32, "!%s",
                             result->payload.stateUpdateI.topic);
                    oledTerminal->appendLine(msg);
                }
                break;
            }
            case MessageType::STATE_UPDATE_CHAR_128: {
                if (homeAssistant.publishStateUpdate(
                        result->payload.stateUpdateS)) {
                    LOG(Serial.println("Published state update successfully"));
                    char msg[32];
                    snprintf(msg, 32, "%s", result->payload.stateUpdateS.topic);
                    oledTerminal->appendLine(msg);
                } else {
                    LOG(Serial.printf(
                        "Failed to publish state update of type %d\n",
                        (uint8_t)result->messageType));
                    char msg[32];
                    snprintf(msg, 32, "!%s",
                             result->payload.stateUpdateS.topic);
                    oledTerminal->appendLine(msg);
                }
                break;
            }
            default: {
                LOG(Serial.printf("Unhandled message type %d",
                                  (uint8_t)result->messageType));
                char msg[24];
                snprintf(msg, 24, "Invalid message type: %d",
                         (uint8_t)result->messageType);

                oledTerminal->appendLine(msg);
                break;
            }
        }

        // Clean up data on heap
        delete result;
        oledTerminal->draw();
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

    if (!oledTerminal->begin()) {
        LOG(Serial.println("Unable to initialize display"));
        while (1) {
            delay(1000);
        }
    }

    oledTerminal->appendLine("Router started.");
    oledTerminal->draw();
}

void loop() {
    currentTime = millis();

    if (currentTime - lastAction >= 1000) {
        LOG(Serial.println(
            "==============================================================="));
        LOG(Serial.printf("HeapMemory: %d\n", ESP.getHeapSize()));
        LOG(Serial.println(
            "==============================================================="));
        lastAction = currentTime;
    }

    mqttClient.check_connection();
    wifiManager.check_connection();

    processSerial();
}
