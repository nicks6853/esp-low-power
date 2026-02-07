#include "origin_builder.h"

#include "message_type.h"

OriginBuilder::OriginBuilder() { this->reset(); }
OriginBuilder::~OriginBuilder() { delete this->_origin; }

/**
 * @brief Adds a name to the origin
 * @param name The name to assign to the origin
 * @return The OriginBuilder instance
 */
OriginBuilder& OriginBuilder::withName(const char* name) {
    if (strlen(name) >= sizeof(this->_origin->name)) {
        Serial.printf("withName - String too long");
        return *this;
    }

    strncpy(this->_origin->name, name, sizeof(this->_origin->name));
    this->_origin->name[sizeof(this->_origin->name) - 1] = '\0';

    return *this;
}

/**
 * @brief Adds a software version to the origin
 * @param softwareVersion The software version to assign to the origin
 * @return The OriginBuilder instance
 */
OriginBuilder& OriginBuilder::withSoftwareVersion(const char* softwareVersion) {
    if (strlen(softwareVersion) >= sizeof(this->_origin->sw)) {
        Serial.printf("withSoftwareVersion - String too long");
        return *this;
    }

    strncpy(this->_origin->sw, softwareVersion, sizeof(this->_origin->sw));
    this->_origin->sw[sizeof(this->_origin->sw) - 1] = '\0';

    return *this;
}

/**
 * @brief Adds a url to the origin
 * @param url The url to assign to the origin
 * @return The OriginBuilder instance
 */
OriginBuilder& OriginBuilder::withUrl(const char* url) {
    if (strlen(url) >= sizeof(this->_origin->url)) {
        Serial.printf("withUrl - String too long");
        return *this;
    }

    strncpy(this->_origin->url, url, sizeof(this->_origin->url));
    this->_origin->url[sizeof(this->_origin->url) - 1] = '\0';

    return *this;
}

/**
 * @brief Allocates a HAOrigin to be built, on the heap
 */
void OriginBuilder::reset() { this->_origin = new HAOrigin(); }

/**
 * @brief Returns the built HAOrigin and resets the builder.
 * @return A pointer to the built HAOrigin on the heap.
 */
HAOrigin* OriginBuilder::build() {
    HAOrigin* result = this->_origin;
    this->reset();
    return result;
}
