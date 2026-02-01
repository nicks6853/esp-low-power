#include <Arduino.h>
#include <ArduinoJson.h>
#include <string.h>

#include "HardwareSerial.h"
#include "config.h"
#include "ha_manager.h"
#include "message_type.h"
#include "mqtt_client.h"
#include "serial_communicator.h"
#include "wifi_manager.h"
#define CAPACITIVE_TOUCH_PIN 4

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MqttClient mqttClient(MQTT_BROKER, MQTT_USER, MQTT_PASSWORD, MQTT_PORT);
HardwareSerial SerialPort(1);
SerialCommunicator serialCommunicator(&SerialPort);
HADiscoveryPayload discoveryPayload;
HAManager homeAssistant(mqttClient);

// TODO: Next, builder pattern to build a HomeAssistant device
// See example in /home/nick/Documents/repo/hierarchical-builder-cpp/

unsigned long touchStart = 0;
bool touched = 0;
touch_value_t touchValue;

void deleteTemperature() {
    Serial.println("TODO: Implement deleteTemperature");
}

void createDevice() {
    Serial.println("Adding device");

    // =========== Initialize Device =========== //
    HADevice device;

    strncpy(device.ids, "something", sizeof(device.ids) - 1);
    device.ids[16] = '\0';

    strncpy(device.name, "Test Device", sizeof(device.name) - 1);
    device.name[255] = '\0';

    strncpy(device.mf, "Nicolas Saulnier Manufacturing", sizeof(device.mf) - 1);
    device.name[255] = '\0';

    strncpy(device.mdl, "Custom Model", sizeof(device.mdl) - 1);
    device.mdl[31] = '\0';

    // =========== Initialize Origin =========== //
    HAOrigin origin;

    strncpy(origin.name, "Nicolas Saulnier", sizeof(origin.name) - 1);
    origin.name[255] = '\0';

    strncpy(origin.sw, "0.0.1-rcbeta", sizeof(origin.sw) - 1);
    origin.sw[31] = '\0';

    strncpy(origin.url, "https://www.nsaulnier.com", sizeof(origin.url) - 1);
    origin.url[255] = '\0';

    // =========== Initialize Component 1 Opts =========== //
    HAComponentOptions cmp1opts;
    strncpy(cmp1opts.p, "sensor", sizeof(cmp1opts.p) - 1);
    cmp1opts.p[31] = '\0';

    strncpy(cmp1opts.dev_cla, "temperature", sizeof(cmp1opts.dev_cla) - 1);
    cmp1opts.dev_cla[31] = '\0';

    strncpy(cmp1opts.uniq_id, "someid_sdfkljsdlsjfd",
            sizeof(cmp1opts.uniq_id) - 1);
    cmp1opts.uniq_id[16] = '\0';

    strncpy(cmp1opts.stat_t, "lskdjflksdjflksdjflksdjkfsdf",
            sizeof(cmp1opts.stat_t) - 1);
    cmp1opts.stat_t[31] = '\0';

    strncpy(cmp1opts.unit_of_meas, "°C", sizeof(cmp1opts.unit_of_meas) - 1);
    cmp1opts.unit_of_meas[31] = '\0';

    // =========== Initializing Components =========== //
    HAComponent cmps[] = {
        //{"temp1cmpID", cmp1opts},
    };

    // =========== Initializing Payload =========== //
    HADiscoveryPayload payload;
    payload.dev = device;
    payload.origin = origin;
    payload.cmps = cmps;
    payload.cmpCount = 0;

    // Send payload to home assistant
    homeAssistant.create_device(payload);
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);
    // SerialPort.begin(ESP_BAUD_RATE, SERIAL_8N1, RX1, TX1);

    // while (!Serial || !SerialPort);
    while (!Serial);

    Serial.println("Starting program");

    wifiManager.connect();
    mqttClient.connect();

    char buffer[255];

    createDevice();
    Serial.print("Relaxing :)");
}

void loop() {
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
            deleteTemperature();
        } else if (totalTouchTime >= 1000) {
            createDevice();
        }

        // Reset touch tracking
        touched = 0;
        touchStart = 0;
    } else if (touched && touchValue <= 50) {
        Serial.printf("\rTouched for: %ldms", millis() - touchStart);
    }

    // mqttClient.publish("testing/something", "payload");
    // uint8_t result = serialCommunicator.read(&discoveryPayload);
    // if (result) {
    //     Serial.printf("Message received: name: %s, url: %s\n",
    //                   discoveryPayload.origin.name,
    //                   discoveryPayload.origin.url);
    // }
}
