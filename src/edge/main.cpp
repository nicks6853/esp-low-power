#include <Arduino.h>

#include "BME280I2C.h"
#include "config.h"
#include "esp32-hal.h"
#include "esplp_espnow_manager.h"
#include "message_type.h"

EspNowManager* espNowManager;
HADiscoveryPayload discoveryPayload;
BME280I2C bme;
unsigned long lastReading = 0;

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
    // espNowManager->autopair();

    unsigned long currentTime = millis();

    if (currentTime - lastReading >= 60000) {
        float temperature = bme.temp();
        float humidity = bme.hum();

        Serial.printf("Sensor read. Temperature: %.2f, Humidity: %.2f",
                      temperature, humidity);

        lastReading = currentTime;
    }
}
