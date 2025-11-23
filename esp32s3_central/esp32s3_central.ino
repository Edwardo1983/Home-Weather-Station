/**
 * @file esp32s3_central.ino
 * @brief Home Weather Station - Central Node (ESP32-S3)
 * @author Senior IoT Developer
 * @version 1.0
 *
 * Multi-display weather station with:
 * - 3x ILI9341 displays with touch input
 * - BME680 environmental sensor
 * - MAX30102 heart rate/SpO2 sensor
 * - SD card data logging
 * - Weather API integration (OpenWeatherMap + Tomorrow.io)
 * - ML-based weather prediction
 * - ESP-NOW communication with remote nodes
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>

#include "config.h"
#include "secrets.h"
#include "display_manager.h"
#include "sensor_manager.h"
#include "espnow_receiver.h"
#include "weather_api.h"
#include "touch_handler.h"
#include "ui_screens.h"
#include "data_logger.h"
#include "ml_predictor.h"
#include "utils.h"

// ============================================================================
// Global Objects
// ============================================================================
DisplayManager displayManager;
SensorManager sensorManager;
ESPNowReceiver espnowReceiver;
WeatherAPI weatherAPI;
TouchHandler touchHandler;
UIScreens uiScreens;
DataLogger dataLogger;
MLPredictor mlPredictor;

// ============================================================================
// Global State
// ============================================================================
struct SystemState {
  unsigned long lastSensorRead = 0;
  unsigned long lastESPNowPoll = 0;
  unsigned long lastWeatherAPI = 0;
  unsigned long lastDisplayRefresh = 0;
  unsigned long lastSDLog = 0;
  unsigned long lastMLPredict = 0;

  bool wifiConnected = false;
  bool sensorReady = false;
  bool espnowReady = false;
  bool sdCardReady = false;

  uint8_t currentDisplay = 0;  // 0, 1, or 2
  bool screenActive = true;
  unsigned long screenOffTime = 0;

  uint8_t currentScreen = 0;  // UI screen type
  bool needsRedraw = true;
} systemState;

// Remote sensor data
struct RemoteSensorData {
  float temp_interior = 0;
  float humidity_interior = 0;
  float temp_exterior = 0;
  float humidity_exterior = 0;
  float pressure_exterior = 0;
  float light_exterior = 0;
  unsigned long last_update_interior = 0;
  unsigned long last_update_exterior = 0;
} remoteSensorData;

// Weather data from APIs
struct WeatherData {
  float currentTemp = 0;
  float feelsLike = 0;
  int humidity = 0;
  float pressure = 0;
  int weatherCode = 0;
  String description = "";
  float windSpeed = 0;
  float uvIndex = 0;

  // 5-day forecast
  float forecastTemp[5] = {0};
  int forecastCondition[5] = {0};
  float forecastRain[5] = {0};

  unsigned long lastUpdate = 0;
} weatherData;

// ============================================================================
// Setup Function
// ============================================================================
void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n\n========================================"));
  Serial.println(F("Home Weather Station - Central Node"));
  Serial.println(F("ESP32-S3 N16R8"));
  Serial.println(F("========================================\n"));

  // Initialize I2C for sensors
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);
  Serial.println(F("[INIT] I2C bus initialized"));

  // Initialize SPI for displays and SD card
  SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI);
  Serial.println(F("[INIT] SPI bus initialized"));

  // Initialize displays
  Serial.println(F("[INIT] Initializing displays..."));
  if (displayManager.begin()) {
    Serial.println(F("[OK] Displays initialized"));
    systemState.sensorReady = true;
  } else {
    Serial.println(F("[ERROR] Display initialization failed"));
    displayManager.showError("Display Init Failed");
  }

  // Initialize local sensors (BME680, MAX30102)
  Serial.println(F("[INIT] Initializing sensors..."));
  if (sensorManager.begin()) {
    Serial.println(F("[OK] Sensors initialized"));
    systemState.sensorReady = true;
  } else {
    Serial.println(F("[ERROR] Sensor initialization failed"));
    displayManager.showError("Sensor Init Failed");
  }

  // Initialize touch handlers
  Serial.println(F("[INIT] Initializing touch inputs..."));
  if (touchHandler.begin()) {
    Serial.println(F("[OK] Touch inputs initialized"));
  } else {
    Serial.println(F("[WARNING] Touch initialization failed - display only mode"));
  }

  // Initialize SD card
  Serial.println(F("[INIT] Initializing SD card..."));
  if (SD.begin(SD_CS)) {
    systemState.sdCardReady = true;
    Serial.println(F("[OK] SD card initialized"));
    dataLogger.begin();
  } else {
    Serial.println(F("[WARNING] SD card not available"));
  }

  // Initialize WiFi (non-blocking)
  if (ENABLE_WIFI) {
    Serial.println(F("[INIT] Initializing WiFi..."));
    initWiFi();
  }

  // Initialize ESP-NOW
  if (ENABLE_ESPNOW) {
    Serial.println(F("[INIT] Initializing ESP-NOW..."));
    if (espnowReceiver.begin()) {
      systemState.espnowReady = true;
      Serial.println(F("[OK] ESP-NOW initialized"));
    } else {
      Serial.println(F("[ERROR] ESP-NOW initialization failed"));
    }
  }

  // Initialize UI
  uiScreens.setDisplayManager(&displayManager);

  // Show startup message
  displayManager.clear(0x0000);
  displayManager.setTextColor(0xFFFF, 0x0000);
  displayManager.setTextSize(2);
  displayManager.setCursor(10, 100);
  displayManager.print("Home Weather");
  displayManager.setCursor(10, 130);
  displayManager.print("Station");
  displayManager.setCursor(10, 170);
  displayManager.setTextSize(1);
  displayManager.print("Initializing...");

  systemState.needsRedraw = true;

  Serial.println(F("[INIT] Setup complete\n"));
}

// ============================================================================
// Main Loop
// ============================================================================
void loop() {
  unsigned long now = millis();

  // Handle screen timeout
  handleScreenTimeout(now);

  // Read local sensors
  if (now - systemState.lastSensorRead >= SENSOR_READ_INTERVAL) {
    readLocalSensors();
    systemState.lastSensorRead = now;
  }

  // Poll remote nodes (ESP-NOW)
  if (now - systemState.lastESPNowPoll >= ESPNOW_POLL_INTERVAL) {
    if (systemState.espnowReady) {
      espnowReceiver.requestUpdate();
      remoteSensorData.last_update_interior = espnowReceiver.getLastInteriorUpdate();
      remoteSensorData.last_update_exterior = espnowReceiver.getLastExteriorUpdate();
    }
    systemState.lastESPNowPoll = now;
  }

  // Fetch weather from APIs
  if (now - systemState.lastWeatherAPI >= WEATHER_API_INTERVAL) {
    if (systemState.wifiConnected) {
      fetchWeatherData();
      systemState.lastWeatherAPI = now;
    }
  }

  // Update ML prediction
  if (now - systemState.lastMLPredict >= ML_PREDICT_INTERVAL) {
    if (ENABLE_ML_PREDICTIONS) {
      mlPredictor.update(sensorManager.getLastData());
    }
    systemState.lastMLPredict = now;
  }

  // Log data to SD card
  if (now - systemState.lastSDLog >= SD_LOG_INTERVAL) {
    if (systemState.sdCardReady && ENABLE_SD_LOGGING) {
      logDataToSD();
      systemState.lastSDLog = now;
    }
  }

  // Update display
  if (now - systemState.lastDisplayRefresh >= DISPLAY_REFRESH_INTERVAL) {
    updateDisplay();
    systemState.lastDisplayRefresh = now;
  }

  // Handle touch input
  if (ENABLE_TOUCHSCREEN && systemState.screenActive) {
    handleTouchInput();
  }

  // Brief delay to prevent watchdog timeout
  yield();
}

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Initialize WiFi connection
 */
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    systemState.wifiConnected = true;
    Serial.println(F("\n[OK] WiFi connected"));
    Serial.print(F("IP: "));
    Serial.println(WiFi.localIP());

    // Initialize weather API
    weatherAPI.setAPIKeys(OPENWEATHERMAP_API_KEY, TOMORROW_IO_API_KEY);
  } else {
    Serial.println(F("\n[WARNING] WiFi connection failed"));
  }
}

/**
 * Read local sensors (BME680, MAX30102)
 */
void readLocalSensors() {
  if (!systemState.sensorReady) return;

  SensorData data = sensorManager.read();

  if (DEBUG_SENSORS) {
    Serial.print(F("[SENSOR] Temp: "));
    Serial.print(data.temperature);
    Serial.print(F("°C, Humidity: "));
    Serial.print(data.humidity);
    Serial.print(F("%, Pressure: "));
    Serial.print(data.pressure);
    Serial.print(F(" hPa, IAQ: "));
    Serial.println(data.iaq);
  }
}

/**
 * Fetch weather data from APIs
 */
void fetchWeatherData() {
  if (!systemState.wifiConnected) return;

  Serial.println(F("[WEATHER] Fetching data from APIs..."));

  // Try OpenWeatherMap first
  if (weatherAPI.fetchCurrentWeather(LATITUDE, LONGITUDE)) {
    CurrentWeather weather = weatherAPI.getCurrentWeather();
    weatherData.currentTemp = weather.temp;
    weatherData.feelsLike = weather.feelsLike;
    weatherData.humidity = weather.humidity;
    weatherData.pressure = weather.pressure;
    weatherData.description = weather.description;
    weatherData.windSpeed = weather.windSpeed;
    weatherData.lastUpdate = millis();

    if (DEBUG_WEATHER_API) {
      Serial.print(F("[WEATHER OK] Temp: "));
      Serial.print(weatherData.currentTemp);
      Serial.print(F("°C, "));
      Serial.println(weatherData.description);
    }
  } else {
    Serial.println(F("[WARNING] Weather API fetch failed"));
  }
}

/**
 * Log data to SD card in CSV format
 */
void logDataToSD() {
  if (!systemState.sdCardReady) return;

  SensorData localData = sensorManager.getLastData();

  CSVRecord record;
  record.timestamp = getISO8601Timestamp();
  record.temp_indoor = localData.temperature;
  record.humidity_indoor = localData.humidity;
  record.pressure = localData.pressure;
  record.iaq = localData.iaq;
  record.temp_outdoor = remoteSensorData.temp_exterior;
  record.humidity_outdoor = remoteSensorData.humidity_exterior;
  record.light = remoteSensorData.light_exterior;

  if (dataLogger.writeRecord(record)) {
    if (DEBUG_SENSORS) Serial.println(F("[LOG] Data logged to SD"));
  }
}

/**
 * Update display with current data
 */
void updateDisplay() {
  if (!systemState.sensorReady) return;

  // Determine which screen to show
  switch (systemState.currentScreen) {
    case 0:  // Home/Main screen
      uiScreens.drawMainScreen(
        sensorManager.getLastData(),
        remoteSensorData,
        weatherData
      );
      break;
    case 1:  // Weather forecast
      uiScreens.drawForecastScreen(weatherData);
      break;
    case 2:  // Heart rate (when active)
      if (ENABLE_HEART_RATE) {
        HeartRateData hrData = sensorManager.getHeartRateData();
        uiScreens.drawHeartRateScreen(hrData);
      }
      break;
    default:
      systemState.currentScreen = 0;
  }
}

/**
 * Handle touch input events
 */
void handleTouchInput() {
  TouchEvent event = touchHandler.getEvent();

  if (event.type == TOUCH_PRESS) {
    systemState.screenActive = true;
    systemState.screenOffTime = millis();

    // Handle button regions
    if (event.display == 0 && event.y > 200) {
      // Top display buttons
      if (event.x < 100) {
        // Previous button
        systemState.currentScreen = (systemState.currentScreen - 1 + 3) % 3;
        systemState.needsRedraw = true;
      } else if (event.x > 220) {
        // Next button
        systemState.currentScreen = (systemState.currentScreen + 1) % 3;
        systemState.needsRedraw = true;
      }
    }

    if (event.display == 2 && event.y > 200) {
      // Bottom display - Special buttons
      if (event.x < 80) {
        // Settings
        systemState.currentScreen = 0;
      } else if (event.x > 240) {
        // Refresh
        systemState.lastWeatherAPI = 0;
      }
    }
  }
}

/**
 * Handle screen timeout (turn off backlight after inactivity)
 */
void handleScreenTimeout(unsigned long now) {
  if (!systemState.screenActive) return;

  if (now - systemState.screenOffTime > DISPLAY_SLEEP_TIME) {
    systemState.screenActive = false;
    displayManager.setBacklight(0);
    Serial.println(F("[DISPLAY] Screen off - timeout"));
  }
}
