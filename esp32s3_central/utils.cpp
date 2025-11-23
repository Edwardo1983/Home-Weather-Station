/**
 * @file utils.cpp
 * @brief Utility functions implementation
 */

#include "utils.h"

String getISO8601Timestamp() {
  // TODO: Integrate with RTC for accurate time
  // For now, return a placeholder based on system millis
  unsigned long ms = millis();
  unsigned long totalSeconds = ms / 1000;

  // Simple counter-based timestamp (not real calendar)
  unsigned long days = totalSeconds / 86400;
  unsigned long hours = (totalSeconds % 86400) / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;

  char buffer[25];
  snprintf(buffer, sizeof(buffer), "2024-01-%02d T%02d:%02d:%02d Z",
           (int)(days % 31) + 1, (int)hours, (int)minutes, (int)seconds);

  return String(buffer);
}

String colorToHex(uint16_t color) {
  char buffer[7];
  uint8_t r = (color >> 11) & 0x1F;
  uint8_t g = (color >> 5) & 0x3F;
  uint8_t b = color & 0x1F;

  // Scale to 8-bit
  r = (r << 3) | (r >> 2);
  g = (g << 2) | (g >> 4);
  b = (b << 3) | (b >> 2);

  snprintf(buffer, sizeof(buffer), "%02X%02X%02X", r, g, b);
  return String(buffer);
}

void printJsonError(const char* error) {
  Serial.print(F("[JSON ERROR] "));
  Serial.println(error);
}

bool ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.println(F("[WiFi] Reconnecting..."));
  WiFi.disconnect(false);  // Don't turn off radio
  WiFi.mode(WIFI_STA);

  // Brief retry
  for (int i = 0; i < 10; i++) {
    delay(500);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(F("[WiFi] Reconnected"));
      return true;
    }
  }

  Serial.println(F("[WiFi] Reconnection failed"));
  return false;
}

String formatBytes(unsigned long bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < 1024 * 1024) {
    return String(bytes / 1024.0, 1) + " KB";
  } else {
    return String(bytes / (1024.0 * 1024.0), 1) + " MB";
  }
}

uint8_t crc8(const uint8_t* data, size_t length) {
  uint8_t crc = 0;

  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x07;
      } else {
        crc = crc << 1;
      }
    }
  }

  return crc;
}

float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

float mapRange(float value, float inMin, float inMax, float outMin, float outMax) {
  float t = (value - inMin) / (inMax - inMin);
  return outMin + (outMax - outMin) * t;
}
