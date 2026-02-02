#include "components_builder.h"

#include <Arduino.h>

#include "config.h"
#include "message_type.h"

ComponentsBuilder::ComponentsBuilder() { this->reset(); }
ComponentsBuilder::~ComponentsBuilder() { delete[] this->_components; }

ComponentBuilder ComponentsBuilder::addComponent(const char* componentId) {
    if (this->_componentsIndex < HA_MAX_COMPONENT_PER_DEVICE) {
        HAComponent& component = this->_components[this->_componentsIndex++];
        component.key = componentId;

        return ComponentBuilder(*this, component.value);
    }

    Serial.printf(
        "addComponent - Overflow!! Too many components. Maximum is %d\n",
        HA_MAX_COMPONENT_PER_DEVICE);
    HAComponent& component = this->_components[HA_MAX_COMPONENT_PER_DEVICE - 1];
    component.key = componentId;
    return ComponentBuilder(*this, component.value);
}

/**
 * @brief Allocates a HAComponent array to be built, on the heap
 */
void ComponentsBuilder::reset() {
    this->_components = new HAComponent[HA_MAX_COMPONENT_PER_DEVICE];
    this->_componentsIndex = 0;
}

/**
 * @brief Returns the built HAComponent array and resets the builder.
 * @return A pointer to the built HAComponent array on the heap.
 */
HAComponent* ComponentsBuilder::build() {
    HAComponent* result = this->_components;
    this->reset();
    return result;
}

ComponentBuilder::ComponentBuilder(ComponentsBuilder& parent,
                                   HAComponentOptions& componentOptions)
    : _componentOptions(componentOptions), _parent(parent) {}

/**
 * @brief Adds a platform to the component
 * @param name The platform to assign to the component
 * @return The ComponentBuilder instance
 */
ComponentBuilder& ComponentBuilder::withPlatform(const char* platform) {
    if (strlen(platform) >= sizeof(this->_componentOptions.p)) {
        Serial.printf("withPlatform - String too long");
        return *this;
    }

    strncpy(this->_componentOptions.p, platform,
            sizeof(this->_componentOptions.p));
    this->_componentOptions.p[sizeof(this->_componentOptions.p) - 1] = '\0';

    return *this;
}

/**
 * @brief Adds a device class to the component
 * @param name The device class to assign to the component
 * @return The ComponentBuilder instance
 */
ComponentBuilder& ComponentBuilder::withDeviceClass(const char* deviceClass) {
    if (strlen(deviceClass) >= sizeof(this->_componentOptions.dev_cla)) {
        Serial.printf("withDeviceClass - String too long");
        return *this;
    }

    strncpy(this->_componentOptions.dev_cla, deviceClass,
            sizeof(this->_componentOptions.dev_cla));
    this->_componentOptions
        .dev_cla[sizeof(this->_componentOptions.dev_cla) - 1] = '\0';

    return *this;
}

/**
 * @brief Adds a unique ID to the component
 * @param name The unique ID to assign to the component
 * @return The ComponentBuilder instance
 */
ComponentBuilder& ComponentBuilder::withUniqueId(const char* uniqueId) {
    if (strlen(uniqueId) >= sizeof(this->_componentOptions.uniq_id)) {
        Serial.printf("withUniqueId - String too long");
        return *this;
    }

    strncpy(this->_componentOptions.uniq_id, uniqueId,
            sizeof(this->_componentOptions.uniq_id));
    this->_componentOptions
        .uniq_id[sizeof(this->_componentOptions.uniq_id) - 1] = '\0';

    return *this;
}

/**
 * @brief Adds a state topic to the component
 * @param name The state topic to assign to the component
 * @return The ComponentBuilder instance
 */
ComponentBuilder& ComponentBuilder::withStateTopic(const char* stateTopic) {
    if (strlen(stateTopic) >= sizeof(this->_componentOptions.stat_t)) {
        Serial.printf("withStateTopic - String too long");
        return *this;
    }

    strncpy(this->_componentOptions.stat_t, stateTopic,
            sizeof(this->_componentOptions.stat_t));
    this->_componentOptions.stat_t[sizeof(this->_componentOptions.stat_t) - 1] =
        '\0';

    return *this;
}

/**
 * @brief Adds a unit of measurement to the component
 * @param name The unit of measurement to assign to the component
 * @return The ComponentBuilder instance
 */
ComponentBuilder& ComponentBuilder::withUnitOfMeasurement(
    const char* unitOfMeasurement) {
    if (strlen(unitOfMeasurement) >=
        sizeof(this->_componentOptions.unit_of_meas)) {
        Serial.printf("withUnitOfMeasurement - String too long");
        return *this;
    }

    strncpy(this->_componentOptions.unit_of_meas, unitOfMeasurement,
            sizeof(this->_componentOptions.unit_of_meas));
    this->_componentOptions
        .unit_of_meas[sizeof(this->_componentOptions.unit_of_meas) - 1] = '\0';

    return *this;
}

/**
 * @brief Returns the built HAComponentOption and resets the builder.
 * @return A pointer to the built HAComponentOption on the heap.
 */
ComponentsBuilder& ComponentBuilder::completeComponent() {
    return this->_parent;
}
