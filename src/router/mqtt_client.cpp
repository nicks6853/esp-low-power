#include "mqtt_client.h"

#include <Arduino.h>

MqttClient::MqttClient(const char* broker, const char* user,
                       const char* password, int port) {
    _mqtt_broker = broker;
    _mqtt_user = user;
    _mqtt_password = password;
    _mqtt_port = port;

    // Set the wifi client on the MQTT client
    _mqtt_client.setClient(_wifi_client);
    _mqtt_client.setServer(_mqtt_broker, _mqtt_port);
}

void MqttClient::connect() {
    Serial.println("Connecting to MQTT...");

    // Increase buffer size to allow sending messages of
    // size up to 1024 bytes (default is 256 bytes)
    _mqtt_client.setBufferSize(1024);

    while (!_mqtt_client.connected()) {
        char client_id[] = "esp-low-power-router";

        if (_mqtt_client.connect(client_id, _mqtt_user, _mqtt_password)) {
            Serial.println("Connected to MQTT!");
            _mqtt_client.publish("some_topic",
                                 "{\"name\": \"esp-low-power-router\"}");
        } else {
            Serial.print("Failed to connect to MQTT! rc=");
            Serial.print(_mqtt_client.state());
            Serial.printf(" try again in %d seconds\n",
                          MQTTCLIENT_RETRY_DELAY_MS);
            delay(MQTTCLIENT_RETRY_DELAY_MS);
        }
    }
}

void MqttClient::check_connection() {
    // Check MQTT connection is still valid
    if (!_mqtt_client.connected()) {
        Serial.println("MQTT not connected! Attempting to reconnect...");
        this->connect();
    }
    _mqtt_client.loop();
}

bool MqttClient::publish(const char* topic, const char* message) {
    Serial.printf("Publishing to topic: %s and message: %s\n", topic, message);
    return _mqtt_client.publish(topic, message);
}
