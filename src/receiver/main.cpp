#include <Arduino.h>
#include <config.h>
#include <constants.h>
#include <message_type.h>

#include "serial_communicator.h"

HAOrigin origin;
HADiscoveryPayload discovery;
SerialCommunicator serialCommunicator(&Serial);

void setup() {
    Serial.begin(ESP_BAUD_RATE);

    // Set the origin configurations
    // strcpy(origin.name, "Nicolas Saulnier");
    // strcpy(origin.sw, "0.0.1");
    // strcpy(origin.url, "https://www.nsaulnier.com");
    // discovery.origin = origin;
}

void loop() {
    // serialCommunicator.write(&discovery);

    delay(2000);
}
