#ifndef ESPNOWMANAGER_H
#define ESPNOWMANAGER_H

#include <Arduino.h>
#include <espnow.h>

#include "esplp_message.h"

/**
 * Enum to track stages of the auto-pair process
 */
enum EspNowPairingStatus {
    PAIR_IDLE,
    PAIR_REQUESTED,
    PAIR_PAIRED,
    PAIR_FAILED,
};

/**
 * Wrapper around the ESP NOW library
 */
class EspNowManager {
   private:
    static EspNowManager* instance;
    static constexpr uint8_t MAX_CHANNEL = 11;
    EspNowPairingStatus pairingStatus = PAIR_IDLE;
    uint8_t wifiChannel = 0;
    uint8_t macAddress[6];
    uint8_t connected = 0;

   public:
    static EspNowManager* getInstance();
    /**
     * Initialize the ESP NOW protocol
     * @return See documentation for esp_now_init
     */
    int init();

    /**
     * Register a callback for ESP NOW receive events.
     * @param cb The callback function.
     * @return See documentation for esp_now_register_recv_cb
     */
    int registerReceiveCallback(esp_now_recv_cb_t cb);

    /**
     * Register a callback for ESP NOW send events.
     * @param cb The callback function.
     * @return See documentation for esp_now_register_send_cb
     */
    int registerSendCallback(esp_now_send_cb_t cb);

    /**
     * Send a message over the ESP NOW protocol
     * @param peer_addr The address of the peer to send to
     * @param message The message to send to the peer
     * @return See documentation for esp_now_send
     */
    int send(u8* peer_addr, const ESPLPMessage* message);

    /**
     * Set the WiFi channel for the ESP device
     * @param channel The channel to set
     * @return A 1 for success and a 0 for error
     */
    int setChannel(uint8_t channel);

    /**
     * Check the current status of autopairing,
     * handles attempting to connect, and return
     * the status.
     * @return The autopairing status of the device
     */
    EspNowPairingStatus autopair();
};

#endif
