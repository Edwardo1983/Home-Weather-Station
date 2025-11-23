/**
 * @file config.h
 * @brief ESP-01S Exterior node configuration
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
// I2C Configuration
// ============================================================================
#define I2C_SDA 0  // GPIO 0 (D3) - use plain GPIO number for ESP8266
#define I2C_SCL 2  // GPIO 2 (D4) - use plain GPIO number for ESP8266
#define I2C_FREQ 400000     // 400kHz

// ============================================================================
// TCA9548A I2C Multiplexer
// ============================================================================
#define TCA9548A_ADDR 0x70  // A0=A1=A2=GND

// Multiplexer Channels
#define MUX_CHANNEL_SENSORS 0  // AHT20 + BMP280
#define MUX_CHANNEL_LIGHT 1    // BH1750

// ============================================================================
// Sensor I2C Addresses
// ============================================================================
#define AHT20_ADDR 0x38      // Temperature/Humidity
#define BMP280_ADDR 0x76     // Can also be 0x77
#define BH1750_ADDR 0x23     // Can also be 0x5C (high address)

// ============================================================================
// Node Identification
// ============================================================================
#define NODE_TYPE "exterior"
#define NODE_ID 2

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
#define WIFI_TIMEOUT 10000

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
#define DEBUG_SENSORS false
#define DEBUG_ESPNOW false

#endif // CONFIG_H
