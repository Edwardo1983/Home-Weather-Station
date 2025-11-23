# Home Weather Station - Complete Deployment Guide

A complete guide for setting up and deploying the Home Weather Station from code generation to operational system.

---

## 📋 Table of Contents

1. [Quick Start](#quick-start)
2. [Detailed Setup](#detailed-setup)
3. [Hardware Assembly](#hardware-assembly)
4. [Software Configuration](#software-configuration)
5. [Deployment Steps](#deployment-steps)
6. [Troubleshooting](#troubleshooting)
7. [First Run](#first-run)

---

## 🚀 Quick Start

For experienced users who just want the essentials:

```bash
# 1. Install libraries
arduino-cli lib install "ArduinoJson" "TFT_eSPI" "Adafruit BME680" \
  "Adafruit AHTX0" "Adafruit BMP280" "MAX30105" "BH1750" \
  "DHT sensor library" "XPT2046_Touchscreen"

# 2. Copy TFT_eSPI setup
cp esp32s3_central/User_Setup.h ~/Arduino/libraries/TFT_eSPI/User_Setups/

# 3. Create and configure secrets
cp esp32s3_central/secrets_template.h esp32s3_central/secrets.h
# Edit secrets.h with WiFi, API keys, MAC addresses

# 4. Compile all nodes
arduino-cli compile --fqbn esp32:esp32:esp32s3 ./esp32s3_central
arduino-cli compile --fqbn esp8266:esp8266:generic ./esp01s_interior
arduino-cli compile --fqbn esp8266:esp8266:generic ./esp01s_exterior

# 5. Upload (repeat for each board)
arduino-cli upload --fqbn esp32:esp32:esp32s3 --port /dev/ttyUSB0 ./esp32s3_central
```

---

## 📚 Detailed Setup

### Prerequisites

**Software:**
- Arduino IDE 1.8.19+ OR arduino-cli 0.20.0+
- Python 3.7+ (optional, for data analysis)
- Git (recommended)

**Hardware:**
- ESP32-S3 DevKitC
- 2x ESP-01S modules
- Sensors (see README.md)
- Micro USB cables (3x)
- 5V power supply (≥2A)
- Breadboard + jumper wires

---

## 🔧 Hardware Assembly

### Central Node (ESP32-S3) Assembly

**Step 1: Mount displays**
1. Place ESP32-S3 on breadboard
2. Connect 3x ILI9341 displays in parallel (SPI bus)
3. Each display gets independent CS pin
4. Connect backlight (GPIO 47) with PWM capable pin

**Step 2: Connect sensors**
1. BME680 on I2C (GPIO 1 SDA, GPIO 2 SCL)
2. MAX30102 on same I2C bus
3. Use 4.7kΩ pull-up resistors on SDA/SCL

**Step 3: Connect touch inputs**
1. 3x XPT2046 touch controllers
2. Share CLK, DIN, DO with display SPI
3. Each has independent CS and IRQ pins

**Step 4: SD card module**
1. Connect to SPI bus (shares CLK, DIN, DO)
2. Independent CS (GPIO 4)
3. Format SD card as FAT32

**Step 5: Power connections**
- Vin: 5V USB or external PSU
- GND: Ground all components
- 3.3V: For sensor VCC (limited to ~50mA from regulator)

### Interior Node (ESP-01S) Assembly

**Simple:** Just DHT22 connected to GPIO 2

```
ESP-01S GPIO 2 ───[4.7kΩ]─── 3.3V
                  └─ DHT22 DATA PIN
          GND ──── DHT22 GND
          3.3V ─── DHT22 VCC
```

### Exterior Node (ESP-01S) Assembly

```
ESP-01S GPIO 0 ──── TCA9548A SDA ──┬── AHT20 SDA
                                   ├── BMP280 SDA
                                   └── BH1750 SDA (CH1)

        GPIO 2 ──── TCA9548A SCL ──┬── All sensors SCL

        [4.7kΩ pull-ups on both lines]
```

---

## 💾 Software Configuration

### Step 1: Install Board Support

```bash
# Arduino CLI
arduino-cli core install esp32:esp32
arduino-cli core install esp8266:esp8266

# Or via Arduino IDE:
# File → Preferences → Additional Boards Manager URLs:
# https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
# http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

### Step 2: Install Libraries

**Using Arduino CLI (recommended):**

```bash
arduino-cli lib install "ArduinoJson" \
  "TFT_eSPI" \
  "Adafruit BME680 Library" \
  "Adafruit AHTX0" \
  "Adafruit BMP280 Library" \
  "MAX30105" \
  "BH1750" \
  "DHT sensor library" \
  "XPT2046_Touchscreen"
```

**Using Arduino IDE:**

Sketch → Include Library → Manage Libraries
Search and install each library

**Verify installation:**

```bash
arduino-cli lib list | grep -E "ArduinoJson|TFT_eSPI|Adafruit|MAX30105|BH1750|DHT|XPT2046"
```

### Step 3: Configure TFT_eSPI

**CRITICAL STEP:** TFT_eSPI requires User_Setup.h in the library directory

**Windows:**
```bash
copy esp32s3_central\User_Setup.h ^
  "%USERPROFILE%\Documents\Arduino\libraries\TFT_eSPI\User_Setups\User_Setup.h"
```

**Linux/macOS:**
```bash
cp esp32s3_central/User_Setup.h \
  ~/Arduino/libraries/TFT_eSPI/User_Setups/User_Setup.h
```

Verify: Check that file exists at correct location

### Step 4: Create secrets.h Files

For **each node**, copy template and fill in credentials:

**Central Node:**
```bash
cp esp32s3_central/secrets_template.h esp32s3_central/secrets.h
```

Edit `esp32s3_central/secrets.h`:
```cpp
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourPassword"
#define OPENWEATHERMAP_API_KEY "your_api_key"
#define TOMORROW_IO_API_KEY "your_api_key"
#define LATITUDE 44.4268
#define LONGITUDE 26.1025
#define ESPNOW_MAC_INTERIOR {0x60, 0x01, 0x94, 0xA0, 0x12, 0x34}
#define ESPNOW_MAC_EXTERIOR {0x60, 0x01, 0x94, 0xA0, 0x12, 0x35}
```

**Interior & Exterior Nodes:**
```bash
cp esp01s_interior/secrets_template.h esp01s_interior/secrets.h
cp esp01s_exterior/secrets_template.h esp01s_exterior/secrets.h
```

Edit both to add:
```cpp
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourPassword"
#define ESPNOW_MAC_CENTRAL {0x94, 0xB9, 0x7E, 0xC0, 0xA1, 0xBC}
```

### Step 5: Get MAC Addresses

**For each node:**

1. Upload a basic test sketch (compile only, don't upload yet)
2. Open Serial Monitor at 115200 baud
3. Power on the board (via USB)
4. Look for: `[INFO] MAC Address: XX:XX:XX:XX:XX:XX`
5. Record the 6 hex pairs

**Example:**
```
[INFO] MAC Address: 94:B9:7E:C0:A1:BC
```

Update `secrets.h` files with actual MAC addresses

---

## 🚢 Deployment Steps

### Step 1: Verification Compile

```bash
# Central node
arduino-cli compile --fqbn esp32:esp32:esp32s3 ./esp32s3_central

# Interior node
arduino-cli compile --fqbn esp8266:esp8266:generic ./esp01s_interior

# Exterior node
arduino-cli compile --fqbn esp8266:esp8266:generic ./esp01s_exterior
```

Expected: "Sketch uses XXX bytes..." with no errors

### Step 2: Identify COM Ports

```bash
# Arduino CLI
arduino-cli board list

# Output should show:
# /dev/ttyUSB0 (or COM3)  "ESP32-S3"
# /dev/ttyUSB1 (or COM4)  "ESP8266"
# /dev/ttyUSB2 (or COM5)  "ESP8266"
```

### Step 3: Upload Central Node

```bash
# Replace COM3 with your actual port
arduino-cli upload --fqbn esp32:esp32:esp32s3 --port COM3 ./esp32s3_central
```

Wait for upload to complete. Watch for reset messages:
```
Uploading to...
Connected, waiting for device...
RAM Used: XXX bytes
Flash Used: XXX bytes
Upload complete!
```

### Step 4: Upload Interior Node

```bash
arduino-cli upload --fqbn esp8266:esp8266:generic --port COM4 ./esp01s_interior
```

### Step 5: Upload Exterior Node

```bash
arduino-cli upload --fqbn esp8266:esp8266:generic --port COM5 ./esp01s_exterior
```

### Step 6: Verify Serial Output

After uploading each node, monitor serial output:

```bash
arduino-cli monitor --port COM3 --config 115200
```

Watch for startup messages:
```
========================================
Home Weather Station - Central Node
ESP32-S3 N16R8
========================================

[INIT] I2C bus initialized
[OK] Displays initialized
[OK] Sensors initialized
[OK] WiFi connected
[OK] ESP-NOW initialized
[INIT] Setup complete
```

---

## 🐛 Troubleshooting

### Compilation Errors

**Error: `fatal error: [library].h: No such file or directory`**

Solution: Install missing library
```bash
arduino-cli lib install "[library name]"
```

**Error: `Board not found`**

Solution: Install board support
```bash
arduino-cli core install esp32:esp32
arduino-cli core install esp8266:esp8266
```

**Error: `exit status 1` (generic)**

Solution: Check output for specific error, then:
1. Try `arduino-cli lib update-index`
2. Delete library and reinstall
3. Check User_Setup.h is in correct location

### Upload Issues

**Error: `Failed to connect to board`**

1. Check COM port: `arduino-cli board list`
2. Try different USB cable
3. Reset board manually during upload
4. Install CH340 driver (if using that chip)

**Error: `Flash write error`**

1. Reduce baud rate: `--upload.speed=115200`
2. Use shorter USB cable
3. Add external USB hub (may help with power)

### Runtime Issues

**Displays don't show anything**

1. [ ] Check SPI pin connections
2. [ ] Check backlight on GPIO 47 PWM
3. [ ] Monitor serial for initialization errors
4. [ ] Verify User_Setup.h in correct location

**WiFi won't connect**

1. [ ] Check SSID/password in secrets.h
2. [ ] Verify WiFi network is visible
3. [ ] Check signal strength (WiFi must be strong)
4. [ ] Monitor serial for WiFi errors

**ESP-NOW not working**

1. [ ] Verify MAC addresses in all secrets.h files
2. [ ] Check nodes powered (not just USB - needs good supply)
3. [ ] Reduce distance between nodes (test <2m)
4. [ ] Monitor serial with `#define DEBUG_ESPNOW true`

**Sensors not reading**

1. [ ] Check I2C pull-ups (4.7kΩ to 3.3V)
2. [ ] Verify sensor I2C addresses
3. [ ] Check sensor power (3.3V, not 5V)
4. [ ] Monitor serial for I2C errors

### Performance Issues

**System runs slow**

- Reduce display refresh rate in config.h
- Increase API update interval (15 min → 30 min)
- Disable unused features

**Display flickers**

- Check power supply stability
- Reduce SPI clock speed (not recommended)
- Update TFT_eSPI library

**Memory errors (ESP-01S)**

- Disable debug output: `#define DEBUG_SERIAL false`
- Remove unused features
- Consider upgrading to Wemos D1 Mini

---

## 🎯 First Run

### Initial Boot Sequence

**Central Node (ESP32-S3):**

1. Powers on → initialization sequence
2. Displays show "Home Weather Station - Initializing..."
3. I2C sensors scanned
4. WiFi connects (takes 10-20 seconds)
5. ESP-NOW initializes
6. Waits for remote node data

**Remote Nodes (ESP-01S):**

1. Powers on → DHT22/sensors initialize
2. WiFi disabled (just ESP-NOW)
3. Attempts to send data to central
4. Goes to deep sleep after send (5 minutes)

### First Commands

After system boots:

1. **Check serial output** - should see no errors
2. **Verify displays** - should show time and sensor data
3. **Monitor WiFi** - connection status displayed
4. **Test API** - weather data updates every 15 minutes
5. **Check ESP-NOW** - remote node data every 5 minutes

### Data Logging

After first 5 minutes, check SD card:

```
weather.csv:
timestamp,temp_indoor,humidity_indoor,...
2024-01-15T08:00:00Z,22.5,45.2,...
```

---

## 📊 System Monitoring

### Via Serial Monitor

```bash
arduino-cli monitor --port COM3 --config 115200
```

Expected output every 60 seconds:
```
[SENSOR] Reading...
[DHT22] Temp: 22.5°C, Humidity: 45%
[ESPNOW] Sending data...
[WEATHER] Fetching data from APIs...
[LOG] Data logged to SD
```

### Via LED Indicators (Optional)

- **Green LED**: All systems OK
- **Yellow LED**: Warning (sensor not responding)
- **Red LED**: Error (initialization failed)

### Via Display

- Display 1: Time and indoor sensors
- Display 2: Outdoor weather
- Display 3: Extended info and buttons

---

## ✅ Post-Deployment Checklist

After deployment, verify:

- [ ] All 3 nodes powered and connected
- [ ] Central node displays showing data
- [ ] Interior node in secondary room, connected
- [ ] Exterior node outdoors, protected from rain
- [ ] WiFi connected and stable
- [ ] API keys working (weather updates)
- [ ] ESP-NOW communication working (5-min updates)
- [ ] SD card logging (weather.csv created)
- [ ] Serial monitor shows no errors
- [ ] All sensors reading valid data
- [ ] ML predictions showing (after 1st hour)

---

## 📈 Next Steps

### Data Collection Phase

1. Let system run for 2+ weeks to collect training data
2. Export CSV from SD card
3. Analyze weather patterns

### ML Model Training (Optional)

1. Load weather_training_data.csv in Python
2. Train model: pressure trend, humidity, temperature
3. Deploy TensorFlow Lite model back to system

### System Optimization

1. Adjust update intervals based on usage
2. Tune API call frequency
3. Optimize display refresh rate
4. Add more sensors if needed

### Cloud Integration (Future)

1. Sync data to cloud database
2. Build web dashboard
3. Mobile app for monitoring

---

## 🆘 Support Resources

- **Documentation:** See [README.md](README.md)
- **Hardware Diagram:** See [docs/wiring_diagram.md](docs/wiring_diagram.md)
- **API Setup:** See [docs/api_setup.md](docs/api_setup.md)
- **Build Status:** See [BUILD_STATUS.md](BUILD_STATUS.md)

---

## 📝 Changelog

**v1.0.0 - Initial Release**
- Complete 3-node system
- Multi-display support
- Weather API integration
- ML prediction engine
- Data logging to SD card

---

**Last Updated:** 2024-11-22
**Status:** Production Ready
**Support:** See GitHub issues
