#include <Arduino.h>

#include "config.h"
#include "esplp_espnow_manager.h"
#include "esplp_message.h"

EspNowManager* espNowManager;

void callback(uint8_t* mac, uint8_t sendStatus) {
    Serial.printf("Last message sent status: %d", sendStatus);
}

void setup() {
    Serial.begin(ESP_BAUD_RATE);
    while (!Serial);
    Serial.println("Ready!");
    delay(2000);
    espNowManager = EspNowManager::getInstance();

    espNowManager->init();
    espNowManager->registerSendCallback(&callback);
}

void loop() {
    espNowManager->autopair();

    Serial.println("Sleeping 5 seconds...");
    Serial.println("==================================================");
    delay(5000);
}
