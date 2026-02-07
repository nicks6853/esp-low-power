#ifndef ORIGINBUILDER_H
#define ORIGINBUILDER_H

#include "message_type.h"

/**
 * Builder class to build HAOrigin structs
 */
class OriginBuilder {
   private:
    HAOrigin* _origin;

   public:
    explicit OriginBuilder();
    ~OriginBuilder();
    void reset();
    OriginBuilder& withName(const char* name);
    OriginBuilder& withSoftwareVersion(const char* softwareVersion);
    OriginBuilder& withUrl(const char* url);
    HAOrigin* build();
};
#endif
