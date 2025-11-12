#include "util.h"

void printMacAddress(uint8_t* macAddress, size_t size) {
    Serial.print("Mac Address: ");
    for (size_t i = 0; i < size; i++) {
        if (i != 0) {
            Serial.print(":");
        }
        Serial.print(macAddress[i], HEX);
    }
    Serial.println();
}
