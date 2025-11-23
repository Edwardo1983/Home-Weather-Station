/**
 * @file secrets_template.h
 * @brief Template for ESP-01S Exterior node credentials
 * Copy to secrets.h and fill in values
 */

#ifndef SECRETS_H
#define SECRETS_H

// ============================================================================
// WiFi Credentials (optional)
// ============================================================================
#define WIFI_SSID "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

// ============================================================================
// Central Node MAC Address
// ============================================================================
// MAC address of the central ESP32-S3 node
#define ESPNOW_MAC_CENTRAL {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00}

// ============================================================================
// Debugging
// ============================================================================
#define DEBUG_SERIAL true
#define DEBUG_SENSORS false
#define DEBUG_ESPNOW false

#endif // SECRETS_H
