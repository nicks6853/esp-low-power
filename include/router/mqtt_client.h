#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#define MQTTCLIENT_RETRY_DELAY_MS 5000

#include <PubSubClient.h>
#include <WiFiClient.h>

class MqttClient {
    private:
        const char* _mqtt_broker;
        const char* _mqtt_user;
        const char* _mqtt_password;
        int _mqtt_port;

        WiFiClient _wifi_client;
        PubSubClient _mqtt_client;
    public:
        MqttClient(const char* broker, const char* user, const char* password, int port);
        /**
         * Connects to an MQTT broker with the given configurations
         */
        void connect();

        /**
         * Checks that the connection the MQTT broker is still good.
         * Attempts to reconnect if it's not.
         */
        void check_connection();

        /**
         * Publish message to an mqtt topic
         * @param topic The topic to publish to
         * @param message The message to publish to the topic
         * @return A boolean representing success or failure
         */
        bool publish(const char* topic, const char* message);
};
#endif
