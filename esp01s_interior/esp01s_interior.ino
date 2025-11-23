/**
 * @file esp01s_interior.ino
 * @brief Interior Room Sensor Node (ESP-01S with DHT22)
 * @author Senior IoT Developer
 *
 * Simple remote sensor node that:
 * - Reads DHT22 temperature/humidity
 * - Sends data via ESP-NOW to central node
 * - Uses deep sleep to minimize power consumption
 * - Reports status periodically
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <DHT.h>

#include "config.h"
#include "secrets.h"

// ============================================================================
// Global Objects
// ============================================================================
DHT dht(DHT_PIN, DHT22);

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
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n\n========================================"));
  Serial.println(F("ESP-01S Interior Node - DHT22"));
  Serial.println(F("========================================\n"));

  // Initialize GPIO
  pinMode(D0, OUTPUT);  // Status LED (optional)
  digitalWrite(D0, LOW);

  // Initialize DHT22
  if (!initDHT()) {
    Serial.println(F("[ERROR] DHT22 initialization failed"));
    setStatusLED(LED_ERROR);
    delay(5000);
    // Continue anyway
  }

  // Initialize WiFi for ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  // Disable normal WiFi

  // Get this node's MAC address
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
    setStatusLED(LED_ERROR);
    delay(5000);
  }

  // Register send callback
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataReceived);

  // Add central node as peer
  uint8_t centralMAC[] = ESPNOW_MAC_CENTRAL;
  addPeer(centralMAC);

  // Initialize ESP-NOW data structure
  strcpy(sensorData.nodeType, NODE_TYPE);
  sensorData.temperature = 0;
  sensorData.humidity = 0;
  sensorData.pressure = 0;
  sensorData.light = 0;
  sensorData.rssi = 0;
  sensorData.timestamp = millis();

  setStatusLED(LED_OK);
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

    // After sending, prepare for deep sleep
    if (ENABLE_DEEP_SLEEP) {
      Serial.println(F("[SLEEP] Entering deep sleep..."));
      delay(100);  // Let serial flush

      // Deep sleep for 5 minutes
      ESP.deepSleep(DEEP_SLEEP_TIME * 1000);
      // Note: After deep sleep, setup() runs again
    }
  }

  // Brief delay
  delay(100);
}

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Initialize DHT22 sensor
 */
bool initDHT() {
  dht.begin();
  delay(2000);  // DHT22 needs 2 seconds to stabilize

  float temp = dht.readTemperature();

  if (isnan(temp)) {
    Serial.println(F("[ERROR] DHT22 not responding"));
    return false;
  }

  Serial.println(F("[OK] DHT22 initialized"));
  return true;
}

/**
 * Read sensor data
 */
void readSensors() {
  // Read DHT22
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    if (DEBUG_DHT) {
      Serial.println(F("[WARNING] DHT22 read failed"));
    }
    setStatusLED(LED_WARNING);
    return;
  }

  sensorData.temperature = temp;
  sensorData.humidity = humidity;
  sensorData.timestamp = millis();

  if (DEBUG_DHT) {
    Serial.print(F("[DHT22] Temp: "));
    Serial.print(temp);
    Serial.print(F("°C, Humidity: "));
    Serial.print(humidity);
    Serial.println(F("%"));
  }

  setStatusLED(LED_OK);
}

/**
 * Send data via ESP-NOW
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
    setStatusLED(LED_ERROR);
  }
}

/**
 * Add peer for ESP-NOW communication
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

/**
 * ESP-NOW receive callback (for future commands)
 */
void onDataReceived(uint8_t* mac_addr, uint8_t* data, uint8_t data_len) {
  if (DEBUG_ESPNOW) {
    Serial.print(F("[ESPNOW] Received "));
    Serial.print(data_len);
    Serial.println(F(" bytes"));
  }

  // Can be used for receiving commands from central node
}

/**
 * Set LED status indicator
 * D0 LED (or built-in LED)
 * HIGH = LED on, LOW = LED off (depends on LED wiring)
 */
enum LEDStatus { LED_OK = 0, LED_WARNING = 1, LED_ERROR = 2 };

void setStatusLED(LEDStatus status) {
  // Simple LED blinking pattern
  switch (status) {
    case LED_OK:
      digitalWrite(D0, HIGH);
      break;
    case LED_WARNING:
      digitalWrite(D0, LOW);
      delay(500);
      digitalWrite(D0, HIGH);
      break;
    case LED_ERROR:
      digitalWrite(D0, LOW);
      break;
  }
}
