#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#include "config.h"
#include "receiver_task.h"
#include "sender_task.h"

ReceiverTask* receiverTask;
SenderTask* senderTask;

unsigned long lastAction = 0;
unsigned long currentTime;

/**
 * Receives message chunks from ESPNOW and pushes it to a queue to be processed.
 * @param mac The mac address of the sender
 * @param incomingData The bytes of data received from the sender
 * @param len The length of the incoming data.
 */
void receiveCallback(const uint8_t* mac, const uint8_t* incomingData, int len) {
    Serial.println("Received a message!");

    esp_now_message_t msg;
    memcpy(msg.mac_addr, mac, 6);
    memcpy(msg.payload, incomingData, len);
    msg.length = len;

    receiverTask->pushMsg(msg);
}

/**
 * Initialize the ESPNOW settings for this receiver.
 * @return A byte indicating success or failure
 */
uint8_t initializeEspNow() {
    Serial.println("Initializing ESPNOW...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Print mac address
    Serial.print("Mac address: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESPNOW!");
        return 0;
    }

    uint8_t res = esp_now_register_recv_cb(receiveCallback);
    Serial.printf("RESULT FROM REGISTERING CALLBACK: %d", res);

    Serial.println("ESPNOW Initialized!");

    return 1;
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);

    while (!Serial);

    Serial.println("Serial is ready!");

    // Start background task to process espnow message
    receiverTask = new ReceiverTask("ReceiverTask", 2048, 1);

    // Start background task to process HAMessages and send them over serial
    senderTask = new SenderTask("SenderTask", 2048, 1);

    // Start ESPNOW protocol
    initializeEspNow();
}

void loop() {
    currentTime = millis();
    if (currentTime - lastAction > 2000) {
        lastAction = currentTime;
    }
}
