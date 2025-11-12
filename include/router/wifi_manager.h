#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H
#include <Arduino.h>

class WifiManager {
    private:
        char _ssid[256]; // Store SSID with maxlength of 255
        char _password[256]; // Store password with maxlength of 255

        /**
         * Sets the 802.11 WiFi standard that we want to allow
         *
         * Usage:
         * To allow 802.11b, 802.11g and 802.11n (2.4GHz wifi), you can call it like this:
         * wifiManager.setMode(WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N)
         *
         * Note: the WIFI_PROTOCOL_* macros are defined in esp_wifi.h
         *
         * @param protocol_bitmap The bitmap for the permitted protocols
         */
        void setMode(uint8_t protocol_bitmap);

    public:
        WifiManager(const char* ssid, const char* password);

        /**
         * Connect to the given WiFi network.
         * @return A boolean indicating success or failure
         */
        bool connect();

        /**
         * Check that we are still connected to WiFi, if it fails,
         * attempt to reconnect.
         */
        void check_connection();
};

#endif
