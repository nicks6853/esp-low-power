#ifndef DEVICEBUILDER_H
#define DEVICEBUILDER_H

#include "message_type.h"

// Forward declaration of DeviceBuilder to enable access in other builders
class DeviceBuilder;

/**
 * Builder class to build HADevice structs
 */
class DeviceBuilder {
   private:
    HADevice* _device;

   public:
    explicit DeviceBuilder();
    ~DeviceBuilder();
    void reset();
    DeviceBuilder& withIds(const char* ids);
    DeviceBuilder& withName(const char* name);
    DeviceBuilder& withManufacturer(const char* manufacturer);
    DeviceBuilder& withModel(const char* model);
    HADevice* build();
};
#endif
