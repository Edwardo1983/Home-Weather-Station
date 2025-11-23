/**
 * @file espnow_receiver.h
 * @brief ESP-NOW receiver for remote nodes
 */

#ifndef ESPNOW_RECEIVER_H
#define ESPNOW_RECEIVER_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

/**
 * Data structure for ESP-NOW communication
 * Max 250 bytes per packet
 */
typedef struct {
  char nodeType[16];      // "interior" or "exterior"
  float temperature = 0;
  float humidity = 0;
  float pressure = 0;
  float light = 0;
  int32_t rssi = 0;
  unsigned long timestamp = 0;
} ESPNowData;

/**
 * Manages ESP-NOW reception from remote nodes
 */
class ESPNowReceiver {
public:
  ESPNowReceiver();

  /**
   * Initialize ESP-NOW
   * @return true if successful
   */
  bool begin();

  /**
   * Register a peer MAC address
   */
  void addPeer(const uint8_t* macAddr);

  /**
   * Request data update from remote nodes
   */
  void requestUpdate();

  /**
   * Get last received interior node data
   */
  ESPNowData getInteriorData() const { return lastInteriorData; }

  /**
   * Get last received exterior node data
   */
  ESPNowData getExteriorData() const { return lastExteriorData; }

  /**
   * Get timestamp of last interior update
   */
  unsigned long getLastInteriorUpdate() const { return lastInteriorUpdate; }

  /**
   * Get timestamp of last exterior update
   */
  unsigned long getLastExteriorUpdate() const { return lastExteriorUpdate; }

  /**
   * Check if interior node is online
   */
  bool isInteriorOnline() {
    return (millis() - lastInteriorUpdate) < 600000;  // 10 min timeout
  }

  /**
   * Check if exterior node is online
   */
  bool isExteriorOnline() {
    return (millis() - lastExteriorUpdate) < 600000;
  }

private:
  ESPNowData lastInteriorData;
  ESPNowData lastExteriorData;
  unsigned long lastInteriorUpdate = 0;
  unsigned long lastExteriorUpdate = 0;

  /**
   * Callback for received ESP-NOW data
   */
  static void onDataReceive(const uint8_t* mac, const uint8_t* data, int len);

  /**
   * Callback for sent ESP-NOW data
   */
  static void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);
};

#endif // ESPNOW_RECEIVER_H
