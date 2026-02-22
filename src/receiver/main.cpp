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
    LOG(Serial.println("Received a message from ESPNOW!"));

    esp_now_message_t msg;
    memcpy(msg.mac_addr, mac, 6);
    memcpy(msg.payload, incomingData, len);
    msg.length = len;

    receiverTask->pushMsg(msg);
}

/**
 * Callback to handle messages from ESPNOW once they are pieced together and
 * complete. Sends the messages to the sender queue to be sent over the Serial
 * connection
 * @param incomingMsg The incoming message from ESPNOW.
 */
void completeMessageCallback(const HAMessage* incomingMsg) {
    LOG(Serial.println("Adding complete message to the queue..."));
    senderTask->pushMsg(incomingMsg);
}

/**
 * Initialize the ESPNOW settings for this receiver.
 * @return A byte indicating success or failure
 */
uint8_t initializeEspNow() {
    LOG(Serial.println("Initializing ESPNOW..."));

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Print mac address
    LOG(Serial.print("Mac address: "));
    LOG(Serial.println(WiFi.macAddress()));

    if (esp_now_init() != ESP_OK) {
        LOG(Serial.println("Error initializing ESPNOW!"));
        return 0;
    }

    uint8_t res = esp_now_register_recv_cb(receiveCallback);

    if (res != ESP_OK) {
        LOG(Serial.println("Failed to register ESPNOW callback!"));
        return 0;
    }

    LOG(Serial.println("ESPNOW Initialized!"));

    return 1;
}

void setup() {
#if defined(DEBUG) && DEBUG == 1
    Serial.begin(ESP_BAUD_RATE);

    while (!Serial);
#endif

    Serial2.begin(ESP_BAUD_RATE);

    while (!Serial2);

    LOG(Serial.println("Serial is ready!"));

    // Start background task to process espnow message
    receiverTask = new ReceiverTask("ReceiverTask", 2048, 1);
    // Register callback to handle complete messages from espnow
    receiverTask->registerCompleteCallback(&completeMessageCallback);

    // Start background task to process HAMessages and send them over serial
    senderTask = new SenderTask("SenderTask", 2048, 1);

    // Start ESPNOW protocol
    if (!initializeEspNow()) {
        for (;;) {
        }
    }
}

void loop() {
    currentTime = millis();
    if (currentTime - lastAction > 2000) {
        lastAction = currentTime;
        LOG(Serial.printf("Heap size: %d\n", ESP.getFreeHeap()));
    }
}
