#ifndef CONFIG_H
#define CONFIG_H

// ======================== WiFi ======================== //
#define WIFI_SSID "<todo>"
#define WIFI_PASSWORD "<todo>"

// ======================== MQTT ======================== //
#define MQTT_BROKER "<todo>"
#define MQTT_USER "<todo>"
#define MQTT_PASSWORD "<todo>"
#define MQTT_PORT 1883

// ======================== ESP Config ======================== //
#define ESP_BAUD_RATE 115200

// ======================== Home Assistant ======================== //
#define HA_DISCOVERY_PREFIX "homeassistant"
#define HA_MAX_COMPONENT_PER_DEVICE 5

#define XOFF 0x13
#define XON 0x11

#endif
