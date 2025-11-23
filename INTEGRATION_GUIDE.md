# Web Server Integration Guide

Complete instructions for integrating the web server module with existing ESP32-S3 central firmware.

---

## Overview

The web server adds these capabilities to your existing project:

- **HTTP/WebSocket server** for dashboard access
- **Real-time data streaming** via WebSocket
- **Configuration management** with LittleFS
- **OTA firmware updates** via browser
- **REST API** for automation/integration

All while maintaining the existing sensor reading and display functionality!

---

## Files Added

New files in `esp32s3_central/`:

```
├── web_server.h/.cpp              # Main web server
├── websocket_handler.h/.cpp        # Real-time data streaming
├── config_manager.h/.cpp           # Configuration storage
├── ota_handler.h/.cpp              # Firmware updates
└── data/                           # LittleFS web files
    ├── index.html
    ├── config.html
    ├── nodes.html
    ├── logs.html
    ├── style.css
    └── app.js
```

Total: ~2000 additional lines of code (manageable!)

---

## Step 1: Add Includes to esp32s3_central.ino

Add these to the includes section:

```cpp
// Web server modules
#include "web_server.h"
#include "websocket_handler.h"
#include "config_manager.h"
#include "ota_handler.h"
```

---

## Step 2: Create Global Instances

Add these global objects (before setup()):

```cpp
// ============================================================================
// Web Server & Configuration
// ============================================================================

WebServer webServer(80, 81);           // HTTP on 80, WebSocket on 81
ConfigManager configManager;           // Configuration storage
OTAHandler otaHandler;                 // Firmware updates
// wsHandler already defined in websocket_handler.h
```

---

## Step 3: Initialize in setup()

Add web server initialization after existing setup code:

```cpp
void setup() {
    // ... existing setup code ...

    // Initialize configuration from LittleFS
    Serial.println(F("[INIT] Loading configuration..."));
    if (configManager.loadConfig()) {
        Serial.println(F("[OK] Configuration loaded"));
        // Update WiFi credentials if needed
        if (configManager.getWiFiSSID().length() > 0) {
            // WiFi already configured
        }
    } else {
        Serial.println(F("[WARNING] Using default configuration"));
    }

    // Initialize OTA handler
    if (otaHandler.begin()) {
        Serial.println(F("[OK] OTA handler ready"));
    }

    // Initialize web server
    delay(2000);  // Give WiFi time to connect
    Serial.println(F("[INIT] Starting web server..."));

    if (webServer.begin()) {
        // Set pointers to system modules for API access
        webServer.setSensorManager(&sensorManager);
        webServer.setESPNowReceiver(&espnowReceiver);
        webServer.setWeatherAPI(&weatherAPI);
        webServer.setConfigManager(&configManager);
        webServer.setOTAHandler(&otaHandler);

        Serial.println(F("[OK] Web server initialized"));
    } else {
        Serial.println(F("[WARNING] Web server initialization failed"));
    }
}
```

---

## Step 4: Add Periodic Broadcasting in loop()

In the main `loop()` function, add WebSocket broadcasts:

```cpp
void loop() {
    unsigned long now = millis();

    // ... existing sensor reading code ...

    // Broadcast sensor data to WebSocket clients (every 5 seconds)
    static unsigned long lastWebSocketBroadcast = 0;
    if (now - lastWebSocketBroadcast >= 5000) {
        // Build sensor data JSON
        DynamicJsonDocument doc(1024);

        SensorData localData = sensorManager.getLastData();
        doc["indoor_main"]["temperature"] = localData.temperature;
        doc["indoor_main"]["humidity"] = localData.humidity;
        doc["indoor_main"]["pressure"] = localData.pressure;
        doc["indoor_main"]["iaq"] = localData.iaq;

        ESPNowData interiorData = espnowReceiver.getInteriorData();
        doc["indoor_secondary"]["temperature"] = interiorData.temperature;
        doc["indoor_secondary"]["humidity"] = interiorData.humidity;
        doc["indoor_secondary"]["rssi"] = interiorData.rssi;
        doc["indoor_secondary"]["timestamp"] = espnowReceiver.getLastInteriorUpdate();

        ESPNowData exteriorData = espnowReceiver.getExteriorData();
        doc["outdoor"]["temperature"] = exteriorData.temperature;
        doc["outdoor"]["humidity"] = exteriorData.humidity;
        doc["outdoor"]["pressure"] = exteriorData.pressure;
        doc["outdoor"]["light"] = exteriorData.light;
        doc["outdoor"]["timestamp"] = espnowReceiver.getLastExteriorUpdate();

        // Broadcast
        String jsonStr;
        serializeJson(doc, jsonStr);
        DynamicJsonDocument fullMsg(1024);
        fullMsg["type"] = "sensor_update";
        fullMsg["data"] = doc;
        serializeJson(fullMsg, jsonStr);

        webServer.broadcastWebSocketData(jsonStr);
        wsHandler.processQueue();

        lastWebSocketBroadcast = now;
    }

    // ... rest of existing loop code ...
}
```

---

## Step 5: Handle WiFi Configuration Changes

If user changes WiFi via web interface, handle reconnection:

```cpp
// Add to loop() to check for WiFi changes
static unsigned long lastWiFiCheck = 0;
if (now - lastWiFiCheck >= 60000) {  // Check every minute
    String newSSID = configManager.getWiFiSSID();
    static String lastSSID = "";

    if (newSSID != lastSSID && newSSID.length() > 0) {
        Serial.println(F("[WiFi] Configuration changed, reconnecting..."));
        lastSSID = newSSID;

        // Disconnect and reconnect
        WiFi.disconnect();
        delay(1000);
        WiFi.begin(configManager.getWiFiSSID().c_str(),
                   configManager.getWiFiPassword().c_str());
    }

    lastWiFiCheck = now;
}
```

---

## Step 6: Upload LittleFS Files

**CRITICAL STEP!** Without this, web interface won't work.

### Arduino IDE 2.x:

1. Create `data/` folder in sketch directory:
   ```
   ~/Arduino/Home-Weather-Station/esp32s3_central/data/
   ```

2. Copy web files into it:
   - index.html
   - config.html
   - nodes.html
   - logs.html
   - style.css
   - app.js

3. Upload:
   ```
   Tools → ESP32 Sketch Data Upload
   ```

4. Monitor serial for:
   ```
   [OK] LittleFS mounted
   [OK] Static files serving from LittleFS
   ```

### Arduino CLI:

```bash
# Install LittleFS upload tool
python3 -m pip install littlefs-python

# Create LittleFS image from data folder
python3 -m littlefs.mkfs -c esp32s3_central/data/ lfs.bin

# Upload to ESP32 (requires esptool)
python3 -m esptool --chip esp32s3 --port /dev/ttyUSB0 \
    --baud 921600 write_flash 0x3B0000 lfs.bin
```

---

## Step 7: Memory Considerations

The web server adds these memory requirements:

| Module | Flash | RAM | Notes |
|--------|-------|-----|-------|
| web_server.h/.cpp | 25KB | 15KB | Async server infrastructure |
| websocket_handler | 8KB | 8KB | Message queuing |
| config_manager | 12KB | 10KB | JSON config handling |
| ota_handler | 6KB | 5KB | Update service |
| Web files (LittleFS) | 140KB | 0KB | Not in PSRAM, served from storage |
| **Total** | **~51KB code** | **~38KB RAM** | Total esp32s3_central now: ~300KB |

Still well within ESP32-S3 limits (16MB flash, 8MB PSRAM)!

---

## Step 8: Library Installations

Install required libraries before compiling:

```bash
# Arduino CLI
arduino-cli lib install "ESPAsyncWebServer"
arduino-cli lib install "AsyncTCP"
arduino-cli lib install "LittleFS"
arduino-cli lib install "ESPmDNS"

# Or via Arduino IDE:
# Sketch → Include Library → Manage Libraries
# Search and install each one
```

---

## Step 9: Compilation & Upload

Standard compilation process:

```bash
# Compile (with or without upload)
arduino-cli compile --fqbn esp32:esp32:esp32s3 ./esp32s3_central

# Upload
arduino-cli upload --fqbn esp32:esp32:esp32s3 --port COM3 ./esp32s3_central
```

Watch for compilation errors related to:
- Missing library includes (install libraries)
- Syntax errors in added code (check spelling)
- Memory issues (shouldn't happen, but check heap)

---

## Step 10: First Run

After upload and LittleFS file upload:

1. Open Serial Monitor (115200 baud)
2. Watch for messages:
   ```
   [OK] LittleFS mounted
   [OK] Web server started on port 80
   [OK] WebSocket on port 81
   [OK] mDNS: weatherstation.local
   [INFO] Access at: http://192.168.x.x
   ```

3. Access dashboard:
   - Browser: `http://weatherstation.local`
   - Or: `http://192.168.x.x`

4. Verify data:
   - Dashboard shows live sensor data
   - WebSocket working (open F12 → Network → WS)
   - Configuration page loads

---

## Troubleshooting Integration

### "fatal error: web_server.h: No such file or directory"

**Solution**: Check file is in same directory as esp32s3_central.ino

```
esp32s3_central/
├── esp32s3_central.ino
├── web_server.h        ← Must be here
├── web_server.cpp
├── config.h
└── ...
```

### "undefined reference to `WebServer::begin()'"

**Solution**: Make sure all .cpp files are in project folder and will be compiled

### "LittleFS not mounted" or 404 errors

**Solution**:
1. Verify `data/` folder has files
2. Run "ESP32 Sketch Data Upload" tool
3. Check serial: `[OK] LittleFS mounted`
4. If error, try rebuilding LittleFS image

### WebSocket connection fails

**Solution**:
1. Check port 81 accessible (not blocked by firewall)
2. Verify `webServer.begin()` succeeded
3. Check AsyncTCP library installed
4. Try accessing via IP instead of hostname

### Configuration changes don't persist

**Solution**:
1. Verify `configManager.saveConfig()` called
2. Check LittleFS has space (check with `Serial.print(LittleFS.totalBytes() - LittleFS.usedBytes())`)
3. Check config file exists: `LittleFS.exists("/config.json")`

---

## Module Dependencies

```
esp32s3_central.ino
    ├── web_server.h/.cpp
    │   ├── AsyncWebServer (external)
    │   ├── ArduinoJson (already used)
    │   ├── LittleFS (external)
    │   └── ESPmDNS (external)
    │
    ├── websocket_handler.h/.cpp
    │   ├── AsyncWebSocket (via ESPAsyncWebServer)
    │   └── ArduinoJson
    │
    ├── config_manager.h/.cpp
    │   ├── LittleFS
    │   └── ArduinoJson
    │
    ├── ota_handler.h/.cpp
    │   ├── Update (built-in)
    │   └── ArduinoJson
    │
    └── Existing modules (no changes needed):
        ├── sensor_manager
        ├── espnow_receiver
        ├── weather_api
        ├── display_manager
        └── ...
```

No conflicts with existing code!

---

## Optional Enhancements

### 1. Add Logging to SD Card

In websocket_handler.cpp:

```cpp
void WebSocketHandler::broadcastLog(const char* level, const char* module, const char* message) {
    // Also log to SD card
    if (dataLogger.isReady()) {
        dataLogger.logMessage(level, module, message);
    }
    // ... existing code ...
}
```

### 2. Add Authentication

Uncomment in web_server.cpp:

```cpp
// Protect all endpoints with auth
if (!isAuthenticated(request)) {
    request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
    return;
}
```

### 3. Add HTTPS

Requires certificate:

```cpp
// In web_server.cpp (advanced)
AsyncWebServer* server = new AsyncWebServer(443);
server->beginSecure("/certs/cert.pem", "/certs/private.key", ...");
```

### 4. Rate Limiting

Protect API from abuse:

```cpp
// In web_server.cpp
static unsigned long lastAPICall[256];  // Track by IP
// Check request rate before processing
```

---

## Performance Tips

1. **Reduce WebSocket update interval** if network allows
2. **Compress JSON** responses for large datasets
3. **Cache API responses** longer (reduce weather API calls)
4. **Minify JavaScript** if adding more features
5. **Use GZIP compression** for HTTP responses

---

## Disabling Web Server (if needed)

To compile without web server:

1. Remove includes:
   ```cpp
   // #include "web_server.h"
   // #include "websocket_handler.h"
   // etc.
   ```

2. Comment out initialization:
   ```cpp
   // webServer.begin();
   ```

3. Remove WebSocket broadcasts in loop

This frees up ~50KB flash and ~38KB RAM if needed.

---

## Testing Checklist

- [ ] LittleFS files uploaded successfully
- [ ] Libraries installed and compiled
- [ ] ESP32-S3 compiled without errors
- [ ] Firmware uploaded to board
- [ ] Serial monitor shows "[OK] Web server started"
- [ ] Can access `http://weatherstation.local` in browser
- [ ] Dashboard loads and shows sensor data
- [ ] WebSocket connected (no errors in browser console)
- [ ] Configuration can be changed and saved
- [ ] Nodes page shows ESP-NOW devices
- [ ] OTA page accessible
- [ ] Logs page shows system messages
- [ ] All displays still working with original code
- [ ] Data logging to SD card still works

---

## Next Steps After Integration

1. **Test API endpoints** with curl or Postman
2. **Add Home Assistant integration** via REST API
3. **Create external dashboards** consuming WebSocket data
4. **Implement additional web pages** for specific features
5. **Add user authentication** for security
6. **Deploy to production** with static IP configuration

---

**Integration Complete!** ✅

Your Home Weather Station now has a professional web interface while maintaining all original functionality. Users can monitor and configure the system entirely via browser, no laptop required!

---

**Last Updated**: 2024-11-22
**Version**: 1.0.0
**Complexity**: Intermediate (clear integration points)
