/**
 * @file config.h
 * @brief Central node (ESP32-S3) configuration and pin definitions
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "secrets.h"

// ============================================================================
// Board Configuration
// ============================================================================
#define BOARD_ESP32S3_N16R8
#define FLASH_SIZE 16  // MB
#define PSRAM_SIZE 8   // MB

// ============================================================================
// SPI Bus Configuration (Shared for displays and SD card)
// ============================================================================
#define SPI_MOSI GPIO_NUM_11
#define SPI_MISO GPIO_NUM_13
#define SPI_CLK GPIO_NUM_12

// Display control pins (shared)
#define DISPLAY_DC GPIO_NUM_14
#define DISPLAY_RST GPIO_NUM_21
#define DISPLAY_LED GPIO_NUM_47  // Backlight PWM

// Display Chip Select (Individual)
#define DISPLAY_1_CS GPIO_NUM_10  // Top display
#define DISPLAY_2_CS GPIO_NUM_9   // Middle display
#define DISPLAY_3_CS GPIO_NUM_46  // Bottom display

// SD Card
#define SD_CS GPIO_NUM_4

// ============================================================================
// Touch Screen Configuration (XPT2046 - Shared SPI, different CS)
// ============================================================================
// Touch clock, DIN, DO share with display SPI
#define TOUCH_CLK SPI_CLK    // GPIO 12
#define TOUCH_DIN SPI_MOSI   // GPIO 11
#define TOUCH_DO SPI_MISO    // GPIO 13

// Touch chip select (one per display)
#define TOUCH_1_CS GPIO_NUM_3
#define TOUCH_2_CS GPIO_NUM_8
#define TOUCH_3_CS GPIO_NUM_18

// Touch interrupt pins
#define TOUCH_1_IRQ GPIO_NUM_17
#define TOUCH_2_IRQ GPIO_NUM_16
#define TOUCH_3_IRQ GPIO_NUM_15

// ============================================================================
// I2C Bus Configuration (Local Sensors)
// ============================================================================
#define I2C_SDA GPIO_NUM_1
#define I2C_SCL GPIO_NUM_2
#define I2C_FREQ 400000  // 400kHz

// I2C Sensors
#define BME680_ADDR 0x76       // Can also be 0x77
#define MAX30102_ADDR 0x57

// ============================================================================
// Display Configuration
// ============================================================================
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
#define DISPLAY_ROTATION 0

// TFT_eSPI pins (should match SPI defines above)
#define TFT_MOSI SPI_MOSI
#define TFT_MISO SPI_MISO
#define TFT_SCLK SPI_CLK
#define TFT_DC DISPLAY_DC
#define TFT_RST DISPLAY_RST

// ============================================================================
// Update Intervals (milliseconds)
// ============================================================================
#define SENSOR_READ_INTERVAL 60000      // BME680: 60 seconds
#define ESPNOW_POLL_INTERVAL 300000     // Remote nodes: 5 minutes
#define WEATHER_API_INTERVAL 900000     // Weather API: 15 minutes
#define DISPLAY_REFRESH_INTERVAL 1000   // Display: 1 second
#define SD_LOG_INTERVAL 300000          // SD logging: 5 minutes
#define ML_PREDICT_INTERVAL 3600000     // ML prediction: 1 hour

// ============================================================================
// Data Logging
// ============================================================================
#define LOG_FILE_NAME "weather.csv"
#define LOG_FILE_MAX_SIZE 1048576  // 1MB - rotate when exceeded
#define ENABLE_CSV_HEADER true

// ============================================================================
// Weather API Configuration
// ============================================================================
#define WEATHER_API_TIMEOUT 10000  // 10 seconds
#define WEATHER_API_RETRY_COUNT 3
#define WEATHER_API_RETRY_DELAY 2000  // 2 seconds

// OpenWeatherMap endpoints
#define OWM_CURRENT_URL "https://api.openweathermap.org/data/2.5/weather"
#define OWM_FORECAST_URL "https://api.openweathermap.org/data/2.5/forecast"

// Tomorrow.io endpoints
#define TOMORROW_IO_CURRENT_URL "https://api.tomorrow.io/v4/weather/realtime"
#define TOMORROW_IO_FORECAST_URL "https://api.tomorrow.io/v4/weather/forecast"

// ============================================================================
// ESP-NOW Configuration
// ============================================================================
#define ESPNOW_CHANNEL 6
#define ESPNOW_ENCRYPT false  // Set to true with PMK if needed
#define ESPNOW_MAX_RETRIES 3
#define ESPNOW_TIMEOUT 5000  // 5 seconds

// ============================================================================
// Buffer Sizes
// ============================================================================
#define JSON_BUFFER_SIZE 2048
#define WIFI_TIMEOUT 10000

// ============================================================================
// Heart Rate Sensor (MAX30102)
// ============================================================================
#define MAX30102_MODE_OXIMETRY 3
#define MAX30102_SAMPLE_RATE 100  // Hz
#define MAX30102_PULSE_WIDTH 400  // us
#define MAX30102_LED_CURRENT 30   // mA
#define HR_SAMPLE_SIZE 25

// ============================================================================
// Status Indicators
// ============================================================================
#define STATUS_COLOR_GOOD 0x00FF00    // Green
#define STATUS_COLOR_WARNING 0xFFFF00  // Yellow
#define STATUS_COLOR_ERROR 0xFF0000    // Red
#define STATUS_COLOR_INFO 0x00FFFF     // Cyan

// ============================================================================
// UI Layout Constants
// ============================================================================
#define DISPLAY_1_Y_START 0
#define DISPLAY_2_Y_START 240
#define DISPLAY_3_Y_START 480

#define HEADER_HEIGHT 30
#define MARGIN 5

// ============================================================================
// Sensor Thresholds
// ============================================================================
#define TEMP_ALERT_THRESHOLD 30.0  // °C
#define HUMIDITY_ALERT_THRESHOLD 70  // %
#define PRESSURE_ALERT_DROP 5.0  // hPa drop = rain indicator
#define IAQ_ALERT_THRESHOLD 200  // IAQ score

#endif // CONFIG_H
