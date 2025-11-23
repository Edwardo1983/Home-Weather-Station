# Home Weather Station - Multi-Node IoT System

A sophisticated multi-node environmental monitoring system built on ESP32-S3 and ESP8266 microcontrollers. Features include real-time sensor monitoring, weather API integration, ML-based predictions, and data logging for advanced analysis.

## Project Overview

**Home Weather Station** is a comprehensive IoT solution for home environmental monitoring with:

- **Central Hub** (ESP32-S3): 3 synchronized TFT displays, local sensors, ML prediction engine, SD card data logging
- **Interior Node** (ESP-01S): Secondary room temperature/humidity monitoring via WiFi
- **Exterior Node** (ESP-01S): Multi-sensor outdoor weather station with I2C multiplexer

### Key Features

✅ Real-time multi-sensor monitoring
✅ WiFi-based remote nodes (ESP-NOW protocol)
✅ Weather API integration (OpenWeatherMap + Tomorrow.io)
✅ Machine Learning weather prediction (rule-based, upgradeable to TinyML)
✅ 24/7 data logging to SD card (CSV format for external ML training)
✅ Touch-enabled display interface with 3 synchronized screens
✅ Heart rate monitoring (MAX30102 sensor)
✅ Power-efficient deep sleep for remote nodes
✅ Real-time status monitoring and alerts

---

## Hardware Components

### Central Node (ESP32-S3 N16R8)

| Component | Quantity | Purpose |
|-----------|----------|---------|
| ESP32-S3 DevKitC | 1 | Main processor (16MB Flash, 8MB PSRAM) |
| ILI9341 2.8" SPI Display | 3 | TFT screens (320x240 each) |
| XPT2046 Touch Controller | 3 | Touch input for each display |
| BME680 | 1 | Temperature/Humidity/Pressure/IAQ sensor |
| MAX30102 | 1 | Heart rate & SpO2 monitoring |
| SD Card Module | 1 | Data logging |
| Micro SD Card | 1 | 8GB+ for data storage |

### Interior Node (ESP-01S)

| Component | Quantity | Purpose |
|-----------|----------|---------|
| ESP-01S | 1 | WiFi SoC |
| DHT22 | 1 | Temperature/Humidity sensor |
| Pull-up Resistor | 1 | 4.7kΩ for DHT data line |

### Exterior Node (ESP-01S)

| Component | Quantity | Purpose |
|-----------|----------|---------|
| ESP-01S | 1 | WiFi SoC |
| TCA9548A | 1 | I2C Multiplexer (8-channel) |
| AHT20 | 1 | Temperature/Humidity sensor |
| BMP280 | 1 | Barometric pressure sensor |
| BH1750 | 1 | Ambient light sensor |
| Pull-up Resistors | 2 | 4.7kΩ for I2C bus |

---

## Installation Guide

### Prerequisites

- Arduino IDE 1.8.19+ or arduino-cli
- ESP32 Board Support Package (v2.0.0+)
- ESP8266 Board Support Package (v3.0.0+)
- Required libraries (see Libraries section)

### Step 1: Install Board Support Packages

**Option A: Using Arduino IDE**

1. File → Preferences
2. Add to "Additional Boards Manager URLs":
   - `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
3. Tools → Board Manager → Search and install:
   - "esp32" by Espressif Systems (latest)
   - "esp8266" by ESP8266 Community (latest)

**Option B: Using arduino-cli**

```bash
arduino-cli core install esp32:esp32
arduino-cli core install esp8266:esp8266
```

### Step 2: Install Required Libraries

**Using Arduino IDE:**

Sketch → Include Library → Manage Libraries, then search and install:

```
TFT_eSPI              v2.5.0+       (Display driver)
XPT2046_Touchscreen   v1.4.0+       (Touch input)
Adafruit BME680       v4.0.0+       (Air quality sensor)
Adafruit AHTX0        v2.0.0+       (AHT20 humidity sensor)
Adafruit BMP280       v2.6.0+       (Pressure sensor)
BH1750                v1.3.0+       (Light sensor)
DHT sensor library    v1.4.4+       (DHT22 sensor)
MAX30105              v1.1.2+       (Heart rate sensor)
ArduinoJson           v6.21.0+      (JSON parsing)
```

**Using arduino-cli:**

```bash
arduino-cli lib install "TFT_eSPI"
arduino-cli lib install "XPT2046_Touchscreen"
arduino-cli lib install "Adafruit BME680"
arduino-cli lib install "Adafruit AHTX0"
arduino-cli lib install "Adafruit BMP280"
arduino-cli lib install "BH1750"
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "MAX30105"
arduino-cli lib install "ArduinoJson"
```

### Step 3: Configure TFT_eSPI

1. Locate your TFT_eSPI library folder:
   - **Windows**: `%USERPROFILE%\Documents\Arduino\libraries\TFT_eSPI`
   - **macOS**: `~/Documents/Arduino/libraries/TFT_eSPI`
   - **Linux**: `~/Arduino/libraries/TFT_eSPI`

2. Copy `User_Setup.h` from the project to:
   ```
   TFT_eSPI/User_Setups/User_Setup.h
   ```

3. In Arduino IDE, verify it's being used:
   - Tools → Board → Select appropriate board
   - Tools → Port → Select your COM port

### Step 4: Create secrets.h Files

Each node needs a `secrets.h` file with credentials.

**For esp32s3_central:**

1. Open `esp32s3_central/secrets_template.h`
2. Copy to `esp32s3_central/secrets.h`
3. Fill in values:
   - WiFi SSID and password
   - API keys (see [API Setup Guide](docs/api_setup.md))
   - Location coordinates (latitude, longitude)
   - ESP-NOW MAC addresses (from step 5)

```cpp
#define WIFI_SSID "MyNetwork"
#define WIFI_PASSWORD "MyPassword"
#define OPENWEATHERMAP_API_KEY "your_key_here"
#define LATITUDE 44.4268
#define LONGITUDE 26.1025
```

**For esp01s_interior and esp01s_exterior:**

Similar process - copy `secrets_template.h` to `secrets.h` and fill in:
- WiFi SSID/password
- Central node MAC address

### Step 5: Identify MAC Addresses

Each ESP node has a unique MAC address needed for ESP-NOW communication.

**For each node:**

1. Upload the basic sketch (compile successfully)
2. Open Serial Monitor (115200 baud)
3. Look for "MAC Address: XX:XX:XX:XX:XX:XX" in output
4. Record these addresses and update `secrets.h` files

Example output:
```
[INFO] MAC Address: 94:B9:7E:C0:A1:BC
```

### Step 6: Compile and Upload

**Using Arduino IDE:**

1. Select Board: Tools → Board → ESP32-S3 (for central) or Generic ESP8266 (for nodes)
2. Select Port: Tools → Port → COMX
3. Sketch → Verify/Compile (Ctrl+R)
4. Sketch → Upload (Ctrl+U)

**Using arduino-cli:**

```bash
# Central node
arduino-cli compile --fqbn esp32:esp32:esp32s3 ./esp32s3_central
arduino-cli upload --fqbn esp32:esp32:esp32s3 --port COM3 ./esp32s3_central

# Interior node
arduino-cli compile --fqbn esp8266:esp8266:generic ./esp01s_interior
arduino-cli upload --fqbn esp8266:esp8266:generic --port COM4 ./esp01s_interior

# Exterior node
arduino-cli compile --fqbn esp8266:esp8266:generic ./esp01s_exterior
arduino-cli upload --fqbn esp8266:esp8266:generic --port COM5 ./esp01s_exterior
```

---

## Hardware Setup

See [Wiring Diagram](docs/wiring_diagram.md) for detailed pin connections and schematics.

### Quick Summary

**Central Node SPI Bus:**
- MOSI: GPIO 11
- MISO: GPIO 13
- SCK: GPIO 12

**Display Chip Select Pins:**
- Display 1: GPIO 10
- Display 2: GPIO 9
- Display 3: GPIO 46

**Touch IRQ Pins:**
- Touch 1: GPIO 17
- Touch 2: GPIO 16
- Touch 3: GPIO 15

**Local Sensors (I2C):**
- SDA: GPIO 1
- SCL: GPIO 2

---

## Configuration

### Display Settings

Adjust in `config.h`:

```cpp
#define DISPLAY_BRIGHTNESS 200           // 0-255
#define DISPLAY_SLEEP_TIME 300000        // 5 minutes (ms)
#define DISPLAY_ROTATION 0               // 0-3
```

### Update Intervals

```cpp
#define SENSOR_READ_INTERVAL 60000      // 60 seconds
#define ESPNOW_POLL_INTERVAL 300000     // 5 minutes
#define WEATHER_API_INTERVAL 900000     // 15 minutes
#define SD_LOG_INTERVAL 300000          // 5 minutes
#define ML_PREDICT_INTERVAL 3600000     // 1 hour
```

### Feature Flags

Enable/disable features in `secrets.h`:

```cpp
#define ENABLE_WIFI true
#define ENABLE_ESPNOW true
#define ENABLE_SD_LOGGING true
#define ENABLE_ML_PREDICTIONS true
#define ENABLE_TOUCHSCREEN true
#define ENABLE_HEART_RATE true
```

---

## API Configuration

See [API Setup Guide](docs/api_setup.md) for detailed instructions on obtaining API keys.

### OpenWeatherMap

1. Register: https://openweathermap.org/api
2. Get free API key (1000 calls/day)
3. Add to `secrets.h`:
   ```cpp
   #define OPENWEATHERMAP_API_KEY "your_key_here"
   ```

### Tomorrow.io

1. Register: https://www.tomorrow.io/weather-api/
2. Get free API key (500 calls/day)
3. Add to `secrets.h`:
   ```cpp
   #define TOMORROW_IO_API_KEY "your_key_here"
   ```

---

## Data Logging

The system logs all sensor data to SD card in CSV format for external ML model training.

**Log Format:**
```csv
timestamp,temp_indoor,humidity_indoor,temp_outdoor,humidity_outdoor,pressure,light,iaq
2024-01-15T08:30:45Z,22.5,45.2,15.3,60.1,1013.25,450,42
```

**File Location:** `/weather.csv` on SD card

**File Rotation:** Automatic when file exceeds 1MB or date changes

### Export for ML Training

1. Remove SD card from central node
2. Read CSV file on computer
3. Use data to train custom ML models
4. Deploy trained models back to system

---

## ML Weather Prediction

The system uses rule-based prediction (can be upgraded to TinyML):

**Features Analyzed:**
- Pressure trend (falling = rain likely)
- Temperature trend (rapid changes = unstable)
- Humidity levels (high = precipitation possible)
- Light intensity patterns
- Time of day and season

**Outputs:**
- Rain probability (0-100%)
- Temperature trend (-1=falling, 0=stable, 1=rising)
- General weather condition (Sunny, Cloudy, Rainy, Stormy)

**Improvement Path:**
1. Collect 2+ weeks of data
2. Export CSV from SD card
3. Train TensorFlow Lite model on external PC
4. Deploy using TensorFlowLite_ESP32 library

---

## Troubleshooting

### Display Not Showing Anything

- [ ] Check SPI pin connections
- [ ] Verify TFT_eSPI User_Setup.h is in correct location
- [ ] Check backlight connection and PWM
- [ ] Monitor serial output for initialization errors

### WiFi/ESP-NOW Not Working

- [ ] Verify WiFi SSID/password in secrets.h
- [ ] Check MAC addresses match in all secrets.h files
- [ ] Ensure all nodes powered (Vin 5V supply)
- [ ] Reduce distance between nodes (test <2m)

### Sensors Not Reading

- [ ] Check I2C pull-up resistors (4.7kΩ)
- [ ] Verify sensor I2C addresses (use I2C scanner)
- [ ] Check Wire.begin() pin configuration
- [ ] Monitor serial for I2C errors

### SD Card Issues

- [ ] Format card as FAT32
- [ ] Check SD_CS pin connection
- [ ] Verify SPI bus clock frequency
- [ ] Test with known working SD card

### Compilation Errors

- [ ] Update Arduino IDE to latest version
- [ ] Reinstall board packages: `arduino-cli core update-index`
- [ ] Delete library and reinstall: `rm -rf ~/Arduino/libraries/TFT_eSPI`
- [ ] Check library versions match requirements

### Memory Issues (ESP-01S Nodes)

ESP-01S has limited flash (1MB). If compilation fails:

- [ ] Reduce debug output
- [ ] Disable unused features in config.h
- [ ] Consider upgrading to Wemos D1 Mini (4MB)

---

## Performance Benchmarks

| Metric | Value |
|--------|-------|
| Display refresh rate | 1 sec |
| Sensor read rate | 60 sec |
| Remote node polling | 5 min |
| Weather API update | 15 min |
| Data logging | 5 min |
| ML prediction | 1 hour |
| Deep sleep current | <50µA |
| Active current (central) | ~250mA |

---

## File Structure

```
home_weather_station/
├── esp32s3_central/
│   ├── esp32s3_central.ino
│   ├── config.h
│   ├── secrets_template.h
│   ├── display_manager.{h,cpp}
│   ├── sensor_manager.{h,cpp}
│   ├── espnow_receiver.{h,cpp}
│   ├── weather_api.{h,cpp}
│   ├── touch_handler.{h,cpp}
│   ├── ui_screens.{h,cpp}
│   ├── data_logger.{h,cpp}
│   ├── ml_predictor.{h,cpp}
│   ├── utils.{h,cpp}
│   └── User_Setup.h
├── esp01s_interior/
│   ├── esp01s_interior.ino
│   ├── config.h
│   └── secrets_template.h
├── esp01s_exterior/
│   ├── esp01s_exterior.ino
│   ├── config.h
│   └── secrets_template.h
├── data/
│   └── weather_training_data.csv
├── docs/
│   ├── wiring_diagram.md
│   └── api_setup.md
└── README.md
```

---

## License & Credits

This project demonstrates professional embedded systems development practices for IoT weather monitoring. Designed for educational and personal use.

### Key Technologies

- **Arduino Framework** - Microcontroller programming
- **ESP32-S3 & ESP8266** - Microcontroller platforms
- **TFT_eSPI** - Display library
- **ESP-NOW** - Mesh networking protocol
- **OpenWeatherMap & Tomorrow.io** - Weather data APIs
- **ArduinoJson** - JSON processing

---

## Support & Contributing

For issues, feature requests, or improvements:

1. Check existing troubleshooting section
2. Review serial debug output
3. Check GitHub issues: [claude-code issues](https://github.com/anthropics/claude-code/issues)
4. Provide:
   - Arduino IDE/cli version
   - Board type and version
   - Error messages (full stack trace)
   - Steps to reproduce

---

## Future Enhancements

- [ ] Cloud sync for weather data
- [ ] Mobile app for remote monitoring
- [ ] TinyML models for advanced prediction
- [ ] Energy harvesting for outdoor node
- [ ] MQTT integration for broader IoT systems
- [ ] Real-time alerts for extreme conditions
- [ ] Voice interface (Alexa/Google Home integration)
- [ ] Historical data visualization web dashboard

---

**Last Updated:** 2024-11-22
**Project Version:** 1.0.0
**Status:** Production Ready
