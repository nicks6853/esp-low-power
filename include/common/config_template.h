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
#define ROUTER_CTS 19
#define ROUTER_RTS 18
#define RECEIVER_CTS 18
#define RECEIVER_RTS 19

// ======================== Home Assistant ======================== //
#define HA_DISCOVERY_PREFIX "homeassistant"
#define HA_MAX_COMPONENT_PER_DEVICE 5

// ======================== Debug Settings ======================== //
#define DEBUG 1

#if defined(DEBUG) && DEBUG == 1
#define LOG(x) x
#else
#define LOG(x) (void)0
#endif

#endif
