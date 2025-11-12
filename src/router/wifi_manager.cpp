#include <Arduino.h>
#include <WiFi.h>
#include <wifi_manager.h>
#include <esp_wifi.h>

WifiManager::WifiManager(const char* ssid, const char* password) {
    // Store credentials to automatically re-connect
    strncpy(_ssid, ssid, sizeof(_ssid) - 1);
    _ssid[sizeof(_ssid) - 1] = '\0';

    strncpy(_password, password, sizeof(_password) - 1);
    _password[sizeof(_password) - 1] = '\0';
}

void WifiManager::setMode(uint8_t protocol_bitmap) {
    esp_wifi_set_protocol(WIFI_IF_STA, protocol_bitmap);
}

bool WifiManager::connect() {
    Serial.print("Connecting to wifi...");

    // Set the wifi mode to be a client
    WiFi.mode(WIFI_STA);

    // Setting the wifi protocol to 802.11b/g/n manually can help with some issues connecting quickly to
    // Wifi networks with Wifi 5GHz and Wifi 6 on the same SSID as the 2.4GHz band.
    this->setMode(WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);


    WiFi.begin(_ssid, _password);
    Serial.printf("ssid:%s, pass:%s\n", _ssid, _password);

    // Wait until the Wifi is connected
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        Serial.printf("rc=%d", WiFi.status());
    }

    Serial.println("\nConnected to wifi.");
    return true;
}

void WifiManager::check_connection() {
    // Check wifi connection is still valid
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi not connected! Attempting to reconnect...");

        this->connect();
    }
}
