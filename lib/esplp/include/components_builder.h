#ifndef COMPONENTSBUILDER_H
#define COMPONENTSBUILDER_H

#include "message_type.h"

class ComponentsBuilder;

class ComponentBuilder {
   private:
    HAComponentOptions& _componentOptions;
    ComponentsBuilder& _parent;

   public:
    explicit ComponentBuilder(ComponentsBuilder& parent,
                              HAComponentOptions& componentOptions);

    // Allow moving (required for older compilers to return from functions)
    ComponentBuilder(ComponentBuilder&&) = default;
    ComponentBuilder(const ComponentBuilder&) = delete;
    ComponentBuilder& operator=(const ComponentBuilder&) = delete;
    ComponentBuilder& withPlatform(const char* platform);
    ComponentBuilder& withDeviceClass(const char* deviceClass);
    ComponentBuilder& withUniqueId(const char* uniqueId);
    ComponentBuilder& withStateTopic(const char* stateTopic);
    ComponentBuilder& withUnitOfMeasurement(const char* unitOfMeasurement);
    ComponentsBuilder& completeComponent();
};

/**
 * Builder class to build HAComponents structs
 */
class ComponentsBuilder {
   private:
    HAComponent* _components;
    size_t _componentsIndex;

   public:
    /**
     * Builder class to build HAComponent structs
     */

    explicit ComponentsBuilder();
    ~ComponentsBuilder();
    void reset();
    ComponentBuilder addComponent(const char* componentId);
    HAComponent* build();
};
#endif
