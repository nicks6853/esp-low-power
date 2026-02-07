#include "device_builder.h"

#include "message_type.h"

DeviceBuilder::DeviceBuilder() { this->reset(); }

DeviceBuilder::~DeviceBuilder() { delete this->_device; }

DeviceBuilder& DeviceBuilder::withIds(const char* ids) {
    if (strlen(ids) >= sizeof(this->_device->ids)) {
        Serial.printf("withIds - String too long");
        return *this;
    }

    strncpy(this->_device->ids, ids, sizeof(this->_device->ids));
    this->_device->ids[sizeof(this->_device->ids) - 1] = '\0';

    return *this;
}

/**
 * @brief Adds a name to the device
 * @param name The name to assign to the device
 * @return The DeviceBuilder instance
 */
DeviceBuilder& DeviceBuilder::withName(const char* name) {
    if (strlen(name) >= sizeof(this->_device->name)) {
        Serial.printf("withName - String too long");
        return *this;
    }

    strncpy(this->_device->name, name, sizeof(this->_device->name));
    this->_device->name[sizeof(this->_device->name) - 1] = '\0';

    return *this;
}

/**
 * @brief Adds a manufacturer to the device
 * @param name The manufacturer to assign to the device
 * @return The DeviceBuilder instance
 */
DeviceBuilder& DeviceBuilder::withManufacturer(const char* manufacturer) {
    if (strlen(manufacturer) >= sizeof(this->_device->mf)) {
        Serial.printf("withManufacturer - String too long");
        return *this;
    }

    strncpy(this->_device->mf, manufacturer, sizeof(this->_device->mf));
    this->_device->mf[sizeof(this->_device->mf) - 1] = '\0';

    return *this;
}

/**
 * @brief Adds a model to the device
 * @param name The model to assign to the device
 * @return The DeviceBuilder instance
 */
DeviceBuilder& DeviceBuilder::withModel(const char* model) {
    if (strlen(model) >= sizeof(this->_device->mdl)) {
        Serial.printf("withModel - String too long");
        return *this;
    }

    strncpy(this->_device->mdl, model, sizeof(this->_device->mdl));
    this->_device->mdl[sizeof(this->_device->mdl) - 1] = '\0';

    return *this;
}

/**
 * @brief Allocates a HADevice to be built, on the heap
 */
void DeviceBuilder::reset() { this->_device = new HADevice(); }

/**
 * @brief Returns the built HADevice and resets the builder.
 * @return A pointer to the built HADevice on the heap.
 */
HADevice* DeviceBuilder::build() {
    HADevice* result = this->_device;
    this->reset();
    return result;
}
