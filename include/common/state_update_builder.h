#ifndef STATEUPDATEBUILDER_H
#define STATEUPDATEBUILDER_H

#include "message_type.h"

/**
 * Builder class to build HAStateUpdate structs
 */
template <typename T>
class StateUpdateBuilder {
   private:
    HAStateUpdate<T>* _stateUpdate;

   public:
    explicit StateUpdateBuilder() { this->reset(); }

    ~StateUpdateBuilder() { delete this->_stateUpdate; }

    /**
     * @brief Adds a topic to the state update
     * @param topic The topic to assign to the state update
     * @return The StateUpdateBuilder instance
     */
    StateUpdateBuilder<T>& withTopic(const char* topic) {
        if (strlen(topic) >= sizeof(this->_stateUpdate->topic)) {
            Serial.printf("withTopic - String too long");
            return *this;
        }

        strncpy(this->_stateUpdate->topic, topic,
                sizeof(this->_stateUpdate->topic));
        this->_stateUpdate->topic[sizeof(this->_stateUpdate->topic) - 1] = '\0';

        return *this;
    }

    /**
     * @brief Adds a value to the state update
     * @param value The value to assign to the state update
     * @return The StateUpdateBuilder instance
     */
    StateUpdateBuilder<T>& withValue(const T value) {
        this->_stateUpdate->value = value;

        return *this;
    }

    /**
     * @brief Allocates a HAStateUpdate to be built, on the heap
     */
    void reset() { this->_stateUpdate = new HAStateUpdate<T>(); }

    /**
     * @brief Returns the built HAStateUpdate and resets the builder.
     * @return A pointer to the built HAStateUpdate on the heap.
     */
    HAStateUpdate<T>* build() {
        HAStateUpdate<T>* result = this->_stateUpdate;
        this->reset();
        return result;
    }
};
#endif
