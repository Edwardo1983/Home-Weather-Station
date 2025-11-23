/**
 * @file config.h
 * @brief ESP-01S Interior node configuration
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "secrets.h"

// ============================================================================
// Board Configuration
// ============================================================================
#define BOARD_ESP01S
#define FLASH_SIZE 1  // MB

// ============================================================================
// DHT22 Sensor
// ============================================================================
#define DHT_PIN 2  // GPIO 2 (D4 on ESP8266)
#define DHT_TYPE DHT22

// ============================================================================
// Node Identification
// ============================================================================
#define NODE_TYPE "interior"
#define NODE_ID 1

// ============================================================================
// ESP-NOW Configuration
// ============================================================================
#define ESPNOW_CHANNEL 6
#define ESPNOW_ENCRYPT false

// Central node MAC address (where to send data)
#define CENTRAL_MAC ESPNOW_MAC_CENTRAL

// ============================================================================
// Update Intervals (milliseconds)
// ============================================================================
#define SENSOR_READ_INTERVAL 30000  // Read every 30 seconds
#define DATA_SEND_INTERVAL 300000   // Send to central every 5 minutes
#define DEEP_SLEEP_TIME 300000      // 5 minutes deep sleep

// ============================================================================
// Timing Configuration
// ============================================================================
#define WIFI_TIMEOUT 10000  // Timeout for any WiFi operations

// ============================================================================
// Feature Flags
// ============================================================================
#define ENABLE_DEEP_SLEEP true
#define ENABLE_ESPNOW true
#define ENABLE_LED_STATUS true

// ============================================================================
// Debugging
// ============================================================================
#define DEBUG_SERIAL true
#define DEBUG_DHT false
#define DEBUG_ESPNOW false

#endif // CONFIG_H
