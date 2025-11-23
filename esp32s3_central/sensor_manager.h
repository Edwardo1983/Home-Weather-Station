/**
 * @file sensor_manager.h
 * @brief Local sensor management (BME680, MAX30102)
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Adafruit_BME680.h>
#include "config.h"

/**
 * BME680 sensor data structure
 */
struct SensorData {
  float temperature = 0;  // °C
  float humidity = 0;     // %
  float pressure = 0;     // hPa
  float gasResistance = 0;  // Ohm
  uint8_t iaq = 0;        // Indoor Air Quality (0-500)
  unsigned long timestamp = 0;
};

/**
 * Heart rate sensor data (MAX30102)
 */
struct HeartRateData {
  uint8_t heartRate = 0;  // bpm
  uint8_t spo2 = 0;       // %
  bool isValid = false;
  unsigned long timestamp = 0;
};

/**
 * Manages BME680 and MAX30102 sensors
 */
class SensorManager {
public:
  SensorManager();

  /**
   * Initialize sensors
   * @return true if BME680 is ready
   */
  bool begin();

  /**
   * Read sensor data (BME680)
   * @return Latest sensor data
   */
  SensorData read();

  /**
   * Get last read sensor data
   */
  SensorData getLastData() const { return lastData; }

  /**
   * Read heart rate data (MAX30102)
   */
  void updateHeartRate();

  /**
   * Get heart rate data
   */
  HeartRateData getHeartRateData() const { return lastHeartRateData; }

  /**
   * Check if BME680 is ready
   */
  bool isBME680Ready() const { return bme680Ready; }

  /**
   * Check if MAX30102 is ready
   */
  bool isMAX30102Ready() const { return max30102Ready; }

  /**
   * Enable/disable heart rate monitoring
   */
  void setHeartRateEnabled(bool enabled) { heartRateEnabled = enabled; }

private:
  Adafruit_BME680 bme680;
  SensorData lastData;
  HeartRateData lastHeartRateData;

  bool bme680Ready;
  bool max30102Ready;
  bool heartRateEnabled;

  // MAX30102 buffers and state
  uint32_t irBuffer[100];
  uint32_t redBuffer[100];
  int32_t bufferLength;
  int32_t spo2;
  int8_t validSPO2;
  int32_t heartRate;
  int8_t validHeartRate;

  /**
   * Calculate IAQ from BME680 raw data
   */
  uint8_t calculateIAQ(float gasResistance, float humidity);

  /**
   * Initialize MAX30102 sensor
   */
  bool initMAX30102();

  /**
   * Read MAX30102 buffers and calculate HR/SpO2
   */
  void readMAX30102();
};

#endif // SENSOR_MANAGER_H
