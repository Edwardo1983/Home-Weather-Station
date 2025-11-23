/**
 * @file utils.h
 * @brief Utility functions
 */

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

/**
 * Get current timestamp in ISO 8601 format
 * Format: YYYY-MM-DDTHH:MM:SSZ
 */
String getISO8601Timestamp();

/**
 * Convert RGB565 color to hex string
 */
String colorToHex(uint16_t color);

/**
 * Parse JSON error and print to serial
 */
void printJsonError(const char* error);

/**
 * Ensure WiFi connection is active
 */
bool ensureWiFiConnected();

/**
 * Format bytes for display (e.g., "1.5 MB")
 */
String formatBytes(unsigned long bytes);

/**
 * Calculate CRC8 checksum for data
 */
uint8_t crc8(const uint8_t* data, size_t length);

/**
 * Simple linear interpolation
 */
float lerp(float a, float b, float t);

/**
 * Map value from one range to another
 */
float mapRange(float value, float inMin, float inMax, float outMin, float outMax);

#endif // UTILS_H
