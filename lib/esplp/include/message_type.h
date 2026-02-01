#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H
#include <Arduino.h>

/**
 * For more information about the definitions of terms,
 * visit the HomeAssitant MQTT integration documentation.
 * https://www.home-assistant.io/integrations/mqtt/
 */

struct HADevice {
    char type[32] = "DEVICE";
    char ids[17];    // deviceid -- uuid is typically 16 bytes long
    char name[256];  // name
    char mf[256];    // manufacturer
    char mdl[32];    // model
};

struct HAComponentOptions {
    char type[32] = "COMPONENT_OPTIONS";
    char p[32];             // platform
    char dev_cla[32];       // device_class
    char uniq_id[17];       // unique_id -- uuid is typically 16 bytes long
    char stat_t[32];        // state_topic
    char unit_of_meas[32];  // unit_of_measurement
};

struct HAOrigin {
    char type[32] = "ORIGIN";
    char name[256];  // name
    char sw[32];     // sw_version
    char url[256];   // support_url
};

struct HAComponent {
    const char* key;
    HAComponentOptions value;
};

struct HADiscoveryPayload {
    char type[32] = "DISCOVERY_PAYLOAD";
    HADevice dev;
    HAOrigin origin;
    HAComponent* cmps;  // pointer to the start of the cmps array
    size_t cmpCount;    // count of cmps in the array
};

#endif
