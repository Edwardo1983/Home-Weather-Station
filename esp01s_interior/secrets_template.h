/**
 * @file secrets_template.h
 * @brief Template for ESP-01S Interior node credentials
 * Copy to secrets.h and fill in values
 */

#ifndef SECRETS_H
#define SECRETS_H

// ============================================================================
// WiFi Credentials (optional, used only if WiFi features enabled)
// ============================================================================
#define WIFI_SSID "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

// ============================================================================
// Central Node MAC Address
// ============================================================================
// MAC address of the central ESP32-S3 node (where to send ESP-NOW data)
// Format: {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}
#define ESPNOW_MAC_CENTRAL {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00}

// ============================================================================
// Debugging
// ============================================================================
#define DEBUG_SERIAL true
#define DEBUG_DHT false
#define DEBUG_ESPNOW false

#endif // SECRETS_H
