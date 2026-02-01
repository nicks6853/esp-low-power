#include "esplp_espnow_manager.h"

#include <ESP8266WiFi.h>
#include <espnow.h>

#include "esplp_message.h"
#include "util.h"

EspNowManager* EspNowManager::instance = nullptr;

EspNowManager* EspNowManager::getInstance() {
    if (!EspNowManager::instance) {
        Serial.println("Initializing EspNowManager singleton...");
        EspNowManager::instance = new EspNowManager;
    }

    return EspNowManager::instance;
}

int EspNowManager::init() {
    Serial.println("Initializing ESP NOW protocol...");

    this->setChannel(0);
    WiFi.mode(WIFI_STA);  // Set Wifi to station mod
    WiFi.disconnect();    // Ensure WiFi is disconnected

    // Print MAC address
    size_t macSize = sizeof(this->macAddress) / sizeof(this->macAddress)[0];
    WiFi.macAddress(this->macAddress);
    printMacAddress(this->macAddress, macSize);

    // Initialize ESP NOW
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP NOW protocol!");
        return 0;
    }

    this->connected = 1;
    Serial.println("ESP NOW protocol initialized!");
    return 1;
}

int EspNowManager::registerReceiveCallback(esp_now_recv_cb_t cb) {
    Serial.println("Registering receive callback");
    return esp_now_register_recv_cb(cb);
}

int EspNowManager::registerSendCallback(esp_now_send_cb_t cb) {
    Serial.println("Registering send callback");
    return esp_now_register_send_cb(cb);
}

int EspNowManager::send(u8* peer_addr, const ESPLPMessage* message) {
    Serial.printf("Sending data of type %d to ESP NOW peer...\n",
                  message->type);

    uint8_t buffer[sizeof(ESPLPMessage)];
    message->serialize(buffer);

    int result = esp_now_send(peer_addr, buffer, sizeof(ESPLPMessage));
    Serial.println("Data sent to ESP NOW peer!");

    return result;
}

int EspNowManager::setChannel(uint8_t channel) {
    wifi_promiscuous_enable(1);
    bool success = wifi_set_channel(channel);
    wifi_promiscuous_enable(0);

    if (success) {
        this->wifiChannel = 0;
        Serial.printf("Wifi channel set to %d\n", channel);
        return 1;
    }

    Serial.printf("Failed to set the Wifi channel to %d\n", channel);
    return 0;
}

EspNowPairingStatus EspNowManager::autopair() {
    switch (this->pairingStatus) {
        case PAIR_IDLE: {
            Serial.println("PAIR_IDLE - Sending pairing request...");

            // Build autopair message with client information
            ESPLPAutopair autopairMsg;
            memcpy(autopairMsg.mac, this->macAddress, sizeof(this->macAddress));
            autopairMsg.wifiChannel = this->wifiChannel;

            // Build message body to send
            ESPLPMessage messageBody;
            messageBody.type = AUTOPAIR;
            messageBody.size = ESPLPAutopair::size();

            // Copy autopair message data to the message body
            autopairMsg.serialize(messageBody.data);

            // Send the message over ESP NOW on the broadcast MAC address
            uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            this->send(broadcastAddress, &messageBody);

            this->pairingStatus = PAIR_REQUESTED;

            break;
        }
        case PAIR_REQUESTED: {
            // Check how long it's been waiting
            // After timeout, increase the channel
            // and set the status back to PAIR_IDLE
            Serial.println("PAIR_REQUESTED - Not implemented");
            break;
        }
        case PAIR_PAIRED: {
            break;
        }
        case PAIR_FAILED: {
            Serial.println("Failed to pair...");
            break;
        }
        default: {
            Serial.println(
                "Unknown pairing status! Something has gone terribly wrong...");
            break;
        }
    }

    return this->pairingStatus;
}
