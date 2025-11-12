#include <Arduino.h>

#include "config.h"
#include "message_type.h"
#include "mqtt_client.h"
#include "multisensor.h"
#include "serial_communicator.h"
#include "wifi_manager.h"

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MqttClient mqttClient(MQTT_BROKER, MQTT_USER, MQTT_PASSWORD, MQTT_PORT);
SerialCommunicator serialCommunicator(&Serial2);
HADiscoveryPayload discoveryPayload;

MultiSensor multiSensor("sensor_name", "SDLKFJ", "degrees",
                        "some/topic/for/mqtt", "temp", "updated-value");

void setup() {
    Serial.begin(115200);
    Serial2.begin(ESP_BAUD_RATE, SERIAL_8N1, 16, 17);

    while (!Serial || !Serial2);

    Serial.println("Starting program");

    wifiManager.connect();
    mqttClient.connect();

    char buffer[255];
}

void loop() {
    uint8_t result = serialCommunicator.read(&discoveryPayload);
    if (result) {
        Serial.printf("Message received: name: %s, url: %s\n",
                      discoveryPayload.origin.name,
                      discoveryPayload.origin.url);
    }
}
