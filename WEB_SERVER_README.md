# Web Admin Interface - Complete Setup Guide

**Home Weather Station Web Server & Dashboard**

A complete web-based administration interface for the ESP32-S3 central node with real-time monitoring, configuration management, and OTA firmware updates.

---

## 📋 Quick Start

1. **Upload LittleFS Files** (see section below)
2. **Install Libraries** (see section below)
3. **Compile & Upload** firmware
4. **Access Dashboard** at `http://weatherstation.local` or `http://192.168.1.x`

---

## ⚙️ Features Overview

### Dashboard (index.html)
- **System Status**: Uptime, free heap, WiFi signal, temperature
- **Indoor Sensors**: Real-time data from BME680 and DHT22 nodes
- **Outdoor Sensors**: Weather data with API integration
- **ML Predictions**: Rain probability, temperature trends
- **5-Day Forecast**: Visual weather forecast cards
- **Real-time Updates**: WebSocket streaming at 5-second intervals

### Configuration (config.html)
- **WiFi Settings**: SSID, password, static IP configuration
- **Weather APIs**: OpenWeatherMap & Tomorrow.io API key setup
- **Display Settings**: Brightness control, theme selection, timeout
- **Data Logging**: Enable/disable SD logging, interval control
- **OTA Updates**: Upload firmware .bin files directly
- **System Management**: Restart, factory reset, configuration backup

### Nodes Monitor (nodes.html)
- **Remote Nodes Table**: Status, MAC, RSSI, last packet time
- **Node Discovery**: Scan for nearby ESP-NOW devices
- **Connectivity Alerts**: Warning if nodes offline >15 minutes
- **Ping Test**: Test connectivity to individual nodes
- **Add/Remove Nodes**: Manage ESP-NOW peer list

### System Logs (logs.html)
- **Live Log Stream**: Real-time log entries via WebSocket
- **Filtering**: By level (INFO/WARNING/ERROR) and module
- **Statistics**: Count of each log type
- **Export**: Download full logs as text file
- **Auto-scroll**: Automatically follow latest entries

---

## 📦 Required Libraries

### Arduino IDE / arduino-cli

Add these libraries via Manage Libraries or CLI:

```bash
arduino-cli lib install "ESPAsyncWebServer"
arduino-cli lib install "AsyncTCP"
arduino-cli lib install "LittleFS"
arduino-cli lib install "ESPmDNS"
```

### Version Requirements
- **ESPAsyncWebServer**: 1.3.0+
- **AsyncTCP**: 1.1.0+
- **LittleFS**: Built-in with ESP32 core
- **ESPmDNS**: Built-in with ESP32 core
- **ArduinoJson**: Already required (v6.21.0+)

### Verify Installation

```bash
arduino-cli lib list | grep -E "ESPAsync|LittleFS|ESPmDNS"
```

---

## 📂 LittleFS File Structure

Create this folder structure in your project:

```
esp32s3_central/
├── data/                   # LittleFS root
│   ├── index.html         # Dashboard
│   ├── config.html        # Configuration page
│   ├── nodes.html         # Nodes monitor
│   ├── logs.html          # Logs viewer
│   ├── style.css          # Styling
│   └── app.js             # JavaScript logic
├── web_server.h/.cpp      # Web server module
├── websocket_handler.h/.cpp
├── config_manager.h/.cpp
└── ota_handler.h/.cpp
```

All web files (HTML/CSS/JS) must be in the `data/` folder for LittleFS upload.

---

## 🚀 Upload Files to LittleFS

### Method 1: Arduino IDE 2.x (Recommended)

1. Install LittleFS Upload Tool:
   - Tools → Manage Libraries
   - Search: "LittleFS"
   - Install "LittleFS"

2. Create `data` folder in sketch directory:
   ```
   ~/Arduino/Home-Weather-Station/esp32s3_central/data/
   ```

3. Place web files there:
   - index.html
   - config.html
   - nodes.html
   - logs.html
   - style.css
   - app.js

4. Upload to ESP32:
   - Tools → ESP32 Sketch Data Upload
   - Wait for completion (progress bar)
   - Serial monitor shows: "[OK] LittleFS mounted"

### Method 2: PlatformIO (Recommended for advanced users)

PlatformIO has built-in LittleFS support:

```bash
platformio run --target littlefs
platformio run --target upload
```

### Method 3: Manual Upload via Web Interface (After First Flash)

Once firmware is running:

1. Navigate to `http://weatherstation.local/upload` (if implemented)
2. Select files
3. Upload

OR use curl:
```bash
curl -F "file=@index.html" http://192.168.1.x/api/upload/
```

### Method 4: Arduino CLI with esptool.py

```bash
# Create LittleFS image
python3 -m littlefs.mkfs -c data/ lfs.bin

# Upload image
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x3B0000 lfs.bin
```

---

## 🌐 Network Setup

### First Boot: Access Point Mode

If no WiFi configured:

1. ESP32 creates AP: **"WeatherStation-Setup"**
2. Password: **"configure123"**
3. Connect to this network
4. Browser captive portal auto-opens
5. Or navigate to: `http://192.168.4.1`
6. Configure WiFi SSID/password
7. System reboots and connects

### Normal Access: mDNS

Once WiFi configured:

- **Hostname**: `weatherstation.local`
- **URL**: `http://weatherstation.local`
- Requires mDNS support (enabled by default)

### Direct IP Access

Find ESP32 IP from:
- Router DHCP client list
- Serial monitor startup message
- WiFi network scan

Then access: `http://192.168.x.x`

---

## 🔐 Security & Authentication

### Default Credentials
- **Username**: `admin`
- **Password**: `admin`

### First-Time Warning

On first access, browser prompts to change default password:
- Go to Config page
- Change username/password
- Settings saved to LittleFS

### HTTPS (Optional)

For local network, HTTP is sufficient. For external access:

1. Generate self-signed certificate:
   ```bash
   openssl req -x509 -newkey rsa:4096 -nodes -out cert.pem -keyout key.pem -days 365
   ```

2. Include certificate in firmware (requires additional setup)

### IP Whitelist (Optional)

Restrict access to specific IPs in config.json:
```json
{
  "security": {
    "whitelist": ["192.168.1.100", "192.168.1.101"]
  }
}
```

---

## 🎯 Configuration File (config.json)

Stored in LittleFS at `/config.json`:

```json
{
  "wifi": {
    "ssid": "MyNetwork",
    "password": "MyPassword",
    "static_ip": false,
    "ip": "192.168.1.100",
    "gateway": "192.168.1.1",
    "subnet": "255.255.255.0",
    "dns": "8.8.8.8"
  },
  "espnow": {
    "nodes": [
      {
        "name": "Interior",
        "mac": "60:01:94:A0:12:34"
      },
      {
        "name": "Exterior",
        "mac": "60:01:94:A0:12:35"
      }
    ]
  },
  "api": {
    "openweathermap_key": "your_api_key_here",
    "tomorrow_key": "your_api_key_here",
    "latitude": 44.4268,
    "longitude": 26.1025
  },
  "display": {
    "brightness": 80,
    "theme": "dark",
    "timeout_minutes": 10
  },
  "logging": {
    "enabled": true,
    "interval_minutes": 5
  },
  "auth": {
    "username": "admin",
    "password_hash": "hashed_password_here"
  },
  "system": {
    "hostname": "weatherstation",
    "first_boot": true
  }
}
```

---

## 🔗 API Endpoints

All JSON responses support CORS. POST requests use JSON body.

### System

**GET /api/status** - System status
```json
{
  "uptime": 123456,
  "heap": 150000,
  "wifi_signal": -45,
  "temperature": 28.5
}
```

**POST /api/system/restart** - Restart ESP32
```json
{ "success": true, "message": "Restarting..." }
```

**POST /api/system/reset** - Factory reset
```json
{ "success": true, "message": "Resetting..." }
```

### Sensors

**GET /api/sensors** - All sensor readings
```json
{
  "indoor_main": { "temperature": 22.5, "humidity": 45, ... },
  "indoor_secondary": { "temperature": 21.8, "humidity": 48, ... },
  "outdoor": { "temperature": 15.3, "humidity": 60, ... }
}
```

### Nodes

**GET /api/nodes** - Remote node status
```json
{
  "nodes": [
    {
      "name": "Interior",
      "mac": "60:01:94:A0:12:34",
      "online": true,
      "rssi": -50,
      "last_packet": 1704024000000
    }
  ]
}
```

**POST /api/nodes/ping** - Ping a node
```json
{ "mac": "60:01:94:A0:12:34" }
```

**POST /api/nodes/add** - Add node
```json
{ "name": "Kitchen", "mac": "AA:BB:CC:DD:EE:FF" }
```

### Configuration

**GET /api/config/get** - Get current config
```json
{
  "wifi": { "ssid": "MyNetwork" },
  "api": { "latitude": 44.4268, ... },
  "display": { "brightness": 80, ... }
}
```

**POST /api/config/wifi** - Update WiFi
```json
{
  "ssid": "NewNetwork",
  "password": "NewPassword",
  "static_ip": false
}
```

**POST /api/config/display** - Update display settings
```json
{
  "brightness": 100,
  "theme": "light",
  "timeout_minutes": 5
}
```

### Firmware

**POST /api/ota/upload** - Upload firmware
- Form data with file field: `firmware`
- Content-Type: `multipart/form-data`
- Returns progress via WebSocket

**GET /api/ota/status** - OTA status
```json
{ "updating": false, "progress": 0, "error": "" }
```

### Logs

**GET /api/logs?lines=100** - Get last N log lines
```json
{
  "logs": [
    {
      "timestamp": 123456,
      "level": "INFO",
      "module": "WiFi",
      "message": "Connected to network"
    }
  ]
}
```

**GET /api/logs/download** - Download logs as text file

### WiFi Scan

**POST /api/wifi/scan** - Scan available networks
```json
{
  "count": 5,
  "networks": ["Network1", "Network2", "Network3"]
}
```

---

## 🔌 WebSocket Communication

WebSocket at `ws://weatherstation.local:81` for real-time updates.

### Message Types

**Incoming (from client)**
```json
{ "type": "request_sensor_data" }
{ "type": "request_system_status" }
{ "type": "request_forecast" }
{ "type": "request_ml_prediction" }
```

**Outgoing (from server)**
```json
{
  "type": "sensor_update",
  "data": {
    "indoor_main": { ... },
    "outdoor": { ... }
  }
}

{
  "type": "system_status",
  "data": {
    "uptime": 123456,
    "heap": 150000,
    "wifi_signal": -45
  }
}

{
  "type": "log_entry",
  "data": {
    "timestamp": 123456,
    "level": "INFO",
    "module": "WiFi",
    "message": "Connected"
  }
}

{
  "type": "forecast_update",
  "data": {
    "forecast": [
      { "day": "Today", "temp": 22, "condition": "Sunny" }
    ]
  }
}

{
  "type": "ml_prediction",
  "data": {
    "rain_probability": 25.5,
    "temperature_trend": 1,
    "general_condition": "Cloudy"
  }
}
```

---

## 📊 Storage Management

### LittleFS Capacity

ESP32-S3 with 16MB flash:
- **LittleFS Size**: ~3MB (partition 0x3B0000 - 0x500000)
- **Available for files**: ~2.9MB

### Current Web Files Size

```
index.html   ~25 KB
config.html  ~18 KB
nodes.html   ~16 KB
logs.html    ~14 KB
style.css    ~35 KB
app.js       ~32 KB
─────────────────────
Total        ~140 KB (~5% of available)
```

Plenty of space for logs and data!

### Log Rotation

Logs older than 30 days auto-delete via `clearOldLogs()`.

Configuration can change log retention:
```json
{
  "logging": {
    "retention_days": 30,
    "max_size_mb": 500
  }
}
```

---

## 🐛 Troubleshooting

### Can't Access Web Interface

**Problem**: Timeout when accessing `weatherstation.local`

**Solutions**:
1. Check ESP32 is powered on
2. Verify WiFi connection (check serial monitor)
3. Use IP address instead: `http://192.168.x.x`
4. Check mDNS working: `ping weatherstation.local`
5. Restart ESP32: hold BOOT + RST button

### Files Not Loading

**Problem**: 404 errors, white page, or "File not found"

**Solutions**:
1. Verify LittleFS upload succeeded
2. Check `data/` folder exists with files
3. Reupload files to LittleFS
4. Clear browser cache (Ctrl+F5)
5. Check serial monitor: `[OK] LittleFS mounted`

### WebSocket Connection Failed

**Problem**: Console shows "WebSocket connection failed"

**Solutions**:
1. Check WebSocket port 81 is accessible
2. Verify firewall not blocking port 81
3. Check ESP32 Serial monitor for errors
4. Try HTTP instead of HTTPS
5. Refresh page after waiting 10 seconds

### Configuration Changes Not Saved

**Problem**: Settings revert after restart

**Solutions**:
1. Check LittleFS write errors in serial
2. Verify config.json exists: use terminal
3. Check file permissions in LittleFS
4. Factory reset and reconfigure
5. Check SD card isn't full

### OTA Upload Fails

**Problem**: Firmware upload stuck or error

**Solutions**:
1. File must be .bin (not .hex or .elf)
2. File size must match ESP32-S3 flash
3. Verify file not corrupted
4. Check free heap space (>1MB required)
5. Try smaller firmware first
6. Use terminal `esptool.py` directly

---

## 🚀 Performance Optimization

### Memory Usage

Current implementation uses:
- **Heap**: ~150KB (of 512KB available)
- **PSRAM**: ~100KB (of 8MB available)
- **LittleFS**: <1MB (of ~3MB available)

Room for expansion!

### Network Bandwidth

- **Dashboard refresh**: ~2KB per 5 seconds = ~0.4 kbps
- **API calls**: ~5KB per request
- **Logs streaming**: ~1KB per entry
- **OTA upload**: Limited by WiFi, typically 100-500 kbps

### Recommended WiFi Setup

- **Distance**: <10m from router
- **Signal strength**: >-70 dBm
- **Bandwidth**: 2.4GHz (better range than 5GHz)
- **Channel**: Auto or 1/6/11 (avoid overlap)

---

## 🔄 Integration with Main Firmware

The web server runs in parallel with main loop:

```cpp
// In esp32s3_central.ino setup():
webServer.begin();
webServer.setSensorManager(&sensorManager);
webServer.setESPNowReceiver(&espnowReceiver);
webServer.setWeatherAPI(&weatherAPI);
webServer.setConfigManager(&configManager);
webServer.setOTAHandler(&otaHandler);

// In main loop:
// Web server handles all HTTP/WebSocket automatically
// Just call periodic broadcast:
wsHandler.broadcastSensorData(sensorData);
wsHandler.broadcastSystemStatus(uptime, heap, rssi);
wsHandler.processQueue();  // Handle buffered messages
```

---

## 📝 Advanced Features

### Custom API Endpoints

Add to web_server.cpp setupAPIEndpoints():
```cpp
server->on("/api/custom", HTTP_GET, [](AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(256);
    doc["custom_data"] = "value";
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
});
```

### Authentication Enhancement

Basic auth implementation (optional):
```cpp
// Add to web_server.cpp
bool authenticate(AsyncWebServerRequest* request) {
    if (!request->authenticate("admin", "password")) {
        request->requestAuthentication();
        return false;
    }
    return true;
}
```

### Custom Dashboards

Edit HTML/CSS to add:
- Custom charts (Chart.js - inline)
- Different layouts
- Custom color schemes
- Additional data sources
- Mobile-specific views

---

## 📖 Documentation Files Provided

1. **README.md** - Main project documentation
2. **DEPLOYMENT_GUIDE.md** - Hardware setup & deployment
3. **WEB_SERVER_README.md** - This file
4. **docs/wiring_diagram.md** - Hardware connections
5. **docs/api_setup.md** - Weather API configuration

---

## ✅ Verification Checklist

- [ ] Libraries installed (ESPAsyncWebServer, LittleFS, etc)
- [ ] Web files created in `data/` folder
- [ ] LittleFS upload tool configured
- [ ] Files uploaded to LittleFS successfully
- [ ] Firmware compiled without errors
- [ ] Serial monitor shows: "[OK] Web server started"
- [ ] Can access dashboard at `weatherstation.local`
- [ ] Dashboard shows live sensor data
- [ ] WebSocket connected (no console errors)
- [ ] Configuration page loads
- [ ] Can change settings and save
- [ ] OTA upload page accessible
- [ ] Nodes page shows ESP-NOW nodes
- [ ] Logs page shows system messages

---

## 🎓 Learning Resources

- **AsyncWebServer**: https://github.com/me-no-dev/ESPAsyncWebServer
- **ArduinoJson**: https://arduinojson.org/
- **LittleFS**: https://github.com/earlephilhower/littlefs-python
- **ESP32 Documentation**: https://docs.espressif.com/

---

## 📞 Support & Troubleshooting

Check the following in order:

1. **Serial Monitor** (115200 baud) - shows all debug info
2. **Browser Console** (F12) - JavaScript errors
3. **Network Tab** (F12) - HTTP response codes
4. **GitHub Issues** - similar problems?

Common error codes:
- **404**: File not found on LittleFS
- **503**: Service unavailable (module not initialized)
- **500**: Server error (check serial monitor)
- **WebSocket timeout**: Port 81 blocked or WiFi unstable

---

**Last Updated**: 2024-11-22
**Version**: 1.0.0
**Status**: Production Ready
