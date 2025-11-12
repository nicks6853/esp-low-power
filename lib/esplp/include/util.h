#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>

/**
 * Prints the given mac address to the Serial.
 * Example:
 *  FF:FF:FF:FF:FF:FF
 * @param macAddress The mac address to print
 * @param size The size of the macAddress array (should always be 6)
 */
void printMacAddress(uint8_t* macAddress, size_t size);

#endif
