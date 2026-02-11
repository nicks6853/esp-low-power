#ifndef ESPNOWSERVER_H
#define ESPNOWSERVER_H

#include <Arduino.h>
#include <esp_now.h>

class EspNowServer {
   private:
    static EspNowServer* _instance;
    static uint8_t _channel;
    static uint8_t _connected;
    static constexpr uint8_t MAX_CHANNEL = 11;

   public:
    EspNowServer* getInstance();
    uint8_t init();
    uint8_t registerReceiveCallback(esp_now_recv_cb_t cb);
    uint8_t setChannel(uint8_t channel);
};
#endif
