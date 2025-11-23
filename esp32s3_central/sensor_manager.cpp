/**
 * @file sensor_manager.cpp
 * @brief Implementation of sensor management
 */

#include "sensor_manager.h"
#include <Wire.h>

// MAX30102 I2C Registers
#define MAX30102_ADDRESS 0x57
#define MAX30102_IRLED 0x0C
#define MAX30102_RLED 0x0D
#define MAX30102_FIFODATA 0x07

// CRITICAL: Safe I2C write macro with error checking
#define I2C_WRITE(addr, reg, val) do { \
  Wire.beginTransmission(addr); \
  Wire.write(reg); \
  Wire.write(val); \
  if (Wire.endTransmission() != 0) { \
    Serial.println(F("[ERROR] I2C write failed")); \
    return false; \
  } \
} while(0)

SensorManager::SensorManager()
  : bme680Ready(false), max30102Ready(false), heartRateEnabled(false),
    bufferLength(0), spo2(0), validSPO2(0), heartRate(0), validHeartRate(0) {}

bool SensorManager::begin() {
  // Initialize BME680
  if (!bme680.begin(BME680_ADDR, true)) {
    Serial.println(F("[ERROR] BME680 initialization failed"));
    return false;
  }

  // Set BME680 oversampling and filter
  bme680.setTemperatureOversampling(BME680_OS_8X);
  bme680.setHumidityOversampling(BME680_OS_2X);
  bme680.setPressureOversampling(BME680_OS_4X);
  bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme680.setGasHeater(320, 150);  // 320°C for 150ms

  bme680Ready = true;
  Serial.println(F("[OK] BME680 initialized"));

  // Initialize MAX30102
  if (initMAX30102()) {
    max30102Ready = true;
    Serial.println(F("[OK] MAX30102 initialized"));
  } else {
    Serial.println(F("[WARNING] MAX30102 not available"));
  }

  return bme680Ready;
}

SensorData SensorManager::read() {
  SensorData data;
  data.timestamp = millis();

  if (!bme680Ready) return data;

  // Perform BME680 measurement
  unsigned long endTime = bme680.beginReading();
  if (!endTime) {
    Serial.println(F("[ERROR] BME680 read failed"));
    return lastData;
  }

  if (!bme680.endReading()) {
    Serial.println(F("[ERROR] BME680 end read failed"));
    return lastData;
  }

  // Extract measurements
  data.temperature = bme680.temperature;
  data.humidity = bme680.humidity;
  data.pressure = bme680.pressure / 100.0;  // Convert Pa to hPa
  data.gasResistance = bme680.gas_resistance;

  // Calculate IAQ score
  data.iaq = calculateIAQ(data.gasResistance, data.humidity);

  lastData = data;
  return data;
}

uint8_t SensorManager::calculateIAQ(float gasResistance, float humidity) {
  // Simplified IAQ calculation
  // Typical ranges: fresh air ~500kΩ, poor air ~5kΩ
  // IAQ: 0-50 (good), 51-100 (acceptable), 101-150 (little bad), 151-200 (bad), 201+ (very bad)

  if (gasResistance < 100) return 500;  // Very poor
  if (gasResistance < 1000) return 300;
  if (gasResistance < 10000) return 150;
  if (gasResistance < 100000) return 50;

  // Humidity adjustment
  uint8_t iaq = 0;
  if (humidity < 40) iaq = 25;
  else if (humidity > 60) iaq = 30;

  return iaq;
}

bool SensorManager::initMAX30102() {
  // Check if MAX30102 responds
  Wire.beginTransmission(MAX30102_ADDRESS);
  if (Wire.endTransmission() != 0) {
    Serial.println(F("[ERROR] MAX30102 not responding"));
    return false;
  }

  // Reset MAX30102
  I2C_WRITE(MAX30102_ADDRESS, 0x09, 0x40);  // MODE_CONFIG, Reset
  delay(100);

  // Configure for SpO2/HR mode
  I2C_WRITE(MAX30102_ADDRESS, 0x09, 0x03);  // MODE_CONFIG, SpO2 mode

  // Set LED currents
  I2C_WRITE(MAX30102_ADDRESS, MAX30102_IRLED, MAX30102_LED_CURRENT);
  I2C_WRITE(MAX30102_ADDRESS, MAX30102_RLED, MAX30102_LED_CURRENT);

  Serial.println(F("[OK] MAX30102 configured successfully"));
  return true;
}

void SensorManager::updateHeartRate() {
  if (!max30102Ready || !heartRateEnabled) return;

  readMAX30102();
}

void SensorManager::readMAX30102() {
  // Read heart rate and SpO2 from MAX30102
  Wire.beginTransmission(MAX30102_ADDRESS);
  Wire.write(MAX30102_FIFODATA);
  Wire.endTransmission();

  Wire.requestFrom(MAX30102_ADDRESS, 6);

  if (Wire.available() >= 6) {
    uint32_t red = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();
    uint32_t ir = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();

    // Basic heart rate calculation
    // Simple threshold-based peak detection
    if (ir > 50000) {  // Ensure valid finger contact
      // Calculate heart rate from IR LED signal
      // This is simplified - full algorithm uses FFT

      // Simple peak detection (using member variables for proper state management)
      if (ir > lastIR + 5000 && lastIR > 50000) {
        unsigned long now = millis();
        if (lastPeakTime > 0) {
          unsigned long interval = now - lastPeakTime;
          if (interval > 300 && interval < 2000) {  // 30-200 bpm range
            heartRate = 60000 / interval;
            validHeartRate = 1;
          }
        }
        lastPeakTime = now;
      }
      lastIR = ir;

      // Calculate SpO2 from ratio
      // Typical formula: SpO2 = 110 - 25 * (Red/IR ratio)
      if (ir > 0) {
        float ratio = (float)red / ir;
        spo2 = (uint8_t)(110 - 25 * ratio);
        if (spo2 > 100) spo2 = 100;
        if (spo2 > 95) validSPO2 = 1;
      }

      lastHeartRateData.heartRate = heartRate;
      lastHeartRateData.spo2 = spo2;
      lastHeartRateData.isValid = (validHeartRate && validSPO2);
    } else {
      lastHeartRateData.isValid = false;
    }
  }
}
