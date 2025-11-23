/**
 * @file esp01s_exterior.ino
 * @brief Exterior Sensor Node (ESP-01S with TCA9548A multiplexer)
 * @author Senior IoT Developer
 *
 * Multi-sensor outdoor node featuring:
 * - TCA9548A I2C multiplexer
 * - AHT20 temperature/humidity sensor
 * - BMP280 pressure sensor
 * - BH1750 ambient light sensor
 * - ESP-NOW transmission to central node
 * - Deep sleep for power efficiency
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>

#include "config.h"
#include "secrets.h"

// ============================================================================
// Global Objects
// ============================================================================
Adafruit_AHTX0 aht20;
Adafruit_BMP280 bmp280;
BH1750 bh1750;

// ============================================================================
// Data Structure for ESP-NOW
// ============================================================================
typedef struct {
  char nodeType[16];
  float temperature;
  float humidity;
  float pressure;
  float light;
  int32_t rssi;
  unsigned long timestamp;
} ESPNowData;

// ============================================================================
// Global State
// ============================================================================
ESPNowData sensorData;
unsigned long lastSendTime = 0;
unsigned long lastReadTime = 0;
uint8_t sendFailureCount = 0;

// ============================================================================
// Setup Function
// ============================================================================
void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n\n========================================"));
  Serial.println(F("ESP-01S Exterior Node - Multi-Sensor"));
  Serial.println(F("TCA9548A Multiplexer"));
  Serial.println(F("========================================\n"));

  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(I2C_FREQ);
  Serial.println(F("[OK] I2C initialized"));

  // Initialize sensors
  bool sensorsReady = true;

  if (!initTCA9548A()) {
    Serial.println(F("[ERROR] TCA9548A multiplexer not responding"));
    sensorsReady = false;
  }

  if (!initAHT20()) {
    Serial.println(F("[ERROR] AHT20 initialization failed"));
    sensorsReady = false;
  }

  if (!initBMP280()) {
    Serial.println(F("[ERROR] BMP280 initialization failed"));
    sensorsReady = false;
  }

  if (!initBH1750()) {
    Serial.println(F("[ERROR] BH1750 initialization failed"));
    sensorsReady = false;
  }

  // Initialize GPIO
  pinMode(D0, OUTPUT);  // Status LED
  digitalWrite(D0, sensorsReady ? LOW : HIGH);

  // Initialize WiFi for ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  uint8_t macAddr[6];
  WiFi.macAddress(macAddr);
  Serial.print(F("[INFO] MAC Address: "));
  for (int i = 0; i < 6; i++) {
    Serial.print(macAddr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println(F("[ERROR] ESP-NOW initialization failed"));
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(onDataSent);

  // Add central node as peer
  uint8_t centralMAC[] = ESPNOW_MAC_CENTRAL;
  addPeer(centralMAC);

  // Initialize data structure
  strcpy(sensorData.nodeType, NODE_TYPE);
  sensorData.temperature = 0;
  sensorData.humidity = 0;
  sensorData.pressure = 0;
  sensorData.light = 0;
  sensorData.rssi = 0;
  sensorData.timestamp = millis();

  Serial.println(F("[INIT] Setup complete\n"));
}

// ============================================================================
// Main Loop
// ============================================================================
void loop() {
  unsigned long now = millis();

  // Read sensors periodically
  if (now - lastReadTime >= SENSOR_READ_INTERVAL) {
    readSensors();
    lastReadTime = now;
  }

  // Send data periodically
  if (now - lastSendTime >= DATA_SEND_INTERVAL) {
    sendData();
    lastSendTime = now;

    // Prepare for deep sleep
    if (ENABLE_DEEP_SLEEP) {
      Serial.println(F("[SLEEP] Entering deep sleep..."));
      delay(100);
      ESP.deepSleep(DEEP_SLEEP_TIME * 1000);
    }
  }

  delay(100);
}

// ============================================================================
// I2C Multiplexer Control
// ============================================================================

/**
 * Initialize TCA9548A I2C multiplexer
 */
bool initTCA9548A() {
  Wire.beginTransmission(TCA9548A_ADDR);
  uint8_t error = Wire.endTransmission();

  if (error != 0) {
    Serial.print(F("[ERROR] TCA9548A not found at 0x"));
    Serial.println(TCA9548A_ADDR, HEX);
    return false;
  }

  Serial.println(F("[OK] TCA9548A detected"));
  return true;
}

/**
 * Select TCA9548A multiplexer channel
 */
void selectMuxChannel(uint8_t channel) {
  if (channel > 7) return;

  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();

  if (DEBUG_SENSORS) {
    Serial.print(F("[MUX] Selected channel: "));
    Serial.println(channel);
  }
}

// ============================================================================
// Sensor Initialization
// ============================================================================

/**
 * Initialize AHT20 (temperature/humidity)
 */
bool initAHT20() {
  selectMuxChannel(MUX_CHANNEL_SENSORS);

  if (!aht20.begin()) {
    return false;
  }

  Serial.println(F("[OK] AHT20 initialized"));
  return true;
}

/**
 * Initialize BMP280 (pressure)
 */
bool initBMP280() {
  selectMuxChannel(MUX_CHANNEL_SENSORS);

  if (!bmp280.begin(BMP280_ADDR)) {
    return false;
  }

  bmp280.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,   // Temp oversampling
    Adafruit_BMP280::SAMPLING_X16,  // Pressure oversampling
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );

  Serial.println(F("[OK] BMP280 initialized"));
  return true;
}

/**
 * Initialize BH1750 (light sensor)
 */
bool initBH1750() {
  selectMuxChannel(MUX_CHANNEL_LIGHT);

  if (!bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    return false;
  }

  Serial.println(F("[OK] BH1750 initialized"));
  return true;
}

// ============================================================================
// Sensor Reading
// ============================================================================

/**
 * Read all sensor data
 */
void readSensors() {
  Serial.println(F("[SENSOR] Reading..."));

  // Read AHT20
  selectMuxChannel(MUX_CHANNEL_SENSORS);
  sensors_event_t humidity, temp;
  aht20.getEvent(&humidity, &temp);

  sensorData.temperature = temp.temperature;
  sensorData.humidity = humidity.relative_humidity;

  if (DEBUG_SENSORS) {
    Serial.print(F("[AHT20] T="));
    Serial.print(sensorData.temperature);
    Serial.print(F("°C H="));
    Serial.print(sensorData.humidity);
    Serial.println(F("%"));
  }

  // Read BMP280
  selectMuxChannel(MUX_CHANNEL_SENSORS);
  sensorData.pressure = bmp280.readPressure() / 100.0;  // Convert to hPa

  if (DEBUG_SENSORS) {
    Serial.print(F("[BMP280] P="));
    Serial.print(sensorData.pressure);
    Serial.println(F(" hPa"));
  }

  // Read BH1750
  selectMuxChannel(MUX_CHANNEL_LIGHT);
  if (bh1750.measurementReady()) {
    sensorData.light = bh1750.readLightLevel();

    if (DEBUG_SENSORS) {
      Serial.print(F("[BH1750] Light="));
      Serial.print(sensorData.light);
      Serial.println(F(" lux"));
    }
  }

  sensorData.timestamp = millis();
}

// ============================================================================
// ESP-NOW Communication
// ============================================================================

/**
 * Send sensor data via ESP-NOW
 */
void sendData() {
  Serial.println(F("[ESPNOW] Sending data..."));

  uint8_t centralMAC[] = ESPNOW_MAC_CENTRAL;

  int result = esp_now_send(centralMAC, (uint8_t*)&sensorData, sizeof(sensorData));

  if (result == 0) {
    if (DEBUG_ESPNOW) {
      Serial.println(F("[ESPNOW] Send initiated"));
    }
    sendFailureCount = 0;
  } else {
    Serial.print(F("[ERROR] ESP-NOW send failed: "));
    Serial.println(result);
    sendFailureCount++;
  }
}

/**
 * Add peer for ESP-NOW
 */
void addPeer(uint8_t* macAddr) {
  esp_now_add_peer(macAddr, ESP_NOW_ROLE_SLAVE, ESPNOW_CHANNEL, NULL, 0);

  Serial.print(F("[ESPNOW] Added peer: "));
  for (int i = 0; i < 6; i++) {
    Serial.print(macAddr[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

/**
 * ESP-NOW send callback
 */
void onDataSent(uint8_t* mac_addr, uint8_t sendStatus) {
  if (DEBUG_ESPNOW) {
    Serial.print(F("[ESPNOW] Send status: "));
    Serial.println(sendStatus == 0 ? F("Success") : F("Failed"));
  }
}
