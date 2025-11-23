/**
 * @file secrets_template.h
 * @brief Template for sensitive credentials - COPY to secrets.h and fill in values
 * @warning DO NOT commit secrets.h to version control
 *
 * INSTRUCTIONS:
 * 1. Copy this file and rename to secrets.h in the same directory
 * 2. Fill in ALL placeholder values with your actual credentials
 * 3. Add secrets.h to .gitignore
 * 4. Never commit secrets.h
 *
 * To obtain credentials:
 * - OpenWeatherMap: https://openweathermap.org/api
 * - Tomorrow.io: https://www.tomorrow.io/weather-api/
 * - WiFi: Use your network SSID and password
 * - ESP-NOW MAC addresses: See wiring_diagram.md
 */

#ifndef SECRETS_H
#define SECRETS_H

// ============================================================================
// WiFi Credentials
// ============================================================================
#define WIFI_SSID "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

// ============================================================================
// Weather API Keys
// ============================================================================
// Get from: https://openweathermap.org/api
#define OPENWEATHERMAP_API_KEY "YOUR_OPENWEATHERMAP_API_KEY_HERE"

// Get from: https://www.tomorrow.io/weather-api/
#define TOMORROW_IO_API_KEY "YOUR_TOMORROW_IO_API_KEY_HERE"

// ============================================================================
// Location (Latitude, Longitude)
// ============================================================================
// Used by Weather APIs to fetch weather for your location
// Example: Bucharest, Romania = 44.4268, 26.1025
#define LATITUDE 44.4268
#define LONGITUDE 26.1025
#define LOCATION_NAME "Bucharest, RO"

// ============================================================================
// ESP-NOW MAC Addresses
// ============================================================================
// MAC addresses of the wireless nodes
// Format: {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}
// Get MAC addresses by uploading the nodes with serial monitor and reading the output

// Interior node (ESP-01S with DHT22 in secondary room)
#define ESPNOW_MAC_INTERIOR {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01}

// Exterior node (ESP-01S with TCA9548A + sensors)
#define ESPNOW_MAC_EXTERIOR {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x02}

// ============================================================================
// Display Configuration
// ============================================================================
// Brightness level (0-255)
#define DISPLAY_BRIGHTNESS 200
#define DISPLAY_SLEEP_TIME 300000  // 5 minutes before screen off (ms)

// ============================================================================
// Feature Flags
// ============================================================================
#define ENABLE_WIFI true
#define ENABLE_ESPNOW true
#define ENABLE_SD_LOGGING true
#define ENABLE_ML_PREDICTIONS true
#define ENABLE_TOUCHSCREEN true
#define ENABLE_HEART_RATE true

// ============================================================================
// Debugging
// ============================================================================
#define DEBUG_SERIAL true
#define DEBUG_ESPNOW false
#define DEBUG_WEATHER_API false
#define DEBUG_SENSORS false

#endif // SECRETS_H
