# LittleFS Complete Guide for Home Weather Station

Comprehensive guide for understanding, configuring, and managing LittleFS on the ESP32-S3 for the web server.

---

## 📋 Table of Contents

1. [What is LittleFS?](#what-is-littlefs)
2. [Why LittleFS for Web Server?](#why-littlefs-for-web-server)
3. [Storage Layout](#storage-layout)
4. [File Structure](#file-structure)
5. [Upload Methods](#upload-methods)
6. [Configuration Files](#configuration-files)
7. [Code Implementation](#code-implementation)
8. [Performance & Optimization](#performance--optimization)
9. [Maintenance](#maintenance)
10. [Troubleshooting](#troubleshooting)
11. [Backup & Recovery](#backup--recovery)

---

## What is LittleFS?

### Overview

**LittleFS** is a lightweight, fail-safe filesystem designed for embedded systems with limited resources. It provides:

- **Wear leveling**: Distributes writes across flash memory to extend lifespan
- **Fault tolerance**: Survives power loss during write operations
- **Small footprint**: Minimal memory and code overhead
- **File operations**: Standard Unix-like interface (open, read, write, delete)
- **Directories**: Hierarchical file organization (limited depth on ESP32)

### Key Characteristics

| Property | Value |
|----------|-------|
| **Filesystem Type** | Block-based, fail-safe |
| **Block Size** | 4096 bytes |
| **Maximum Files** | ~10,000 (depends on storage) |
| **Maximum File Size** | Up to total storage size |
| **Maximum Filename** | 255 characters |
| **Path Depth** | Limited to reasonable nesting |
| **Wear Leveling** | Automatic |

---

## Why LittleFS for Web Server?

### Requirements Analysis

The web server needs to serve:
- **HTML pages** (4 files, ~70KB total)
- **CSS stylesheet** (1 file, ~35KB)
- **JavaScript** (1 file, ~32KB)
- **Configuration files** (JSON, ~2KB)
- **Temporary files** (OTA uploads, ~400KB max)

### Storage Solution Comparison

| Solution | Pros | Cons |
|----------|------|------|
| **LittleFS** | Efficient, fail-safe, built-in | Max file size limit |
| **SPIFFS** | Older, less efficient | Deprecated, use LittleFS |
| **SD Card** | Large capacity | Slower, requires module |
| **Hardcoded (PROGMEM)** | Fast, no storage limit | Uses Flash memory, inflexible |

**Decision**: Use **LittleFS** for web files + configuration, optionally use **SD Card** for data logging.

---

## Storage Layout

### ESP32-S3 Flash Partition Scheme

The ESP32-S3 has 16MB of flash memory. Default partition layout:

```
┌─────────────────────────────────┐
│  Bootloader (0x0000 - 0x4FFF)   │  ~20KB
│  Boot app (0x5000 - 0x7FFF)     │  ~12KB
├─────────────────────────────────┤
│  Factory App (0x10000 - 0x1FFFFF)│  ~2MB
│  (Firmware goes here)            │
├─────────────────────────────────┤
│  OTADATA (0x200000 - 0x201FFF)  │  ~8KB
│  (OTA state tracking)            │
├─────────────────────────────────┤
│  OTA_0 (0x202000 - 0x3FFFFF)    │  ~2MB
│  (Secondary firmware for OTA)    │
├─────────────────────────────────┤
│  LittleFS (0x400000 - 0xFFFFFF)  │  ~12MB (AVAILABLE)
│  (Web files + config)            │
├─────────────────────────────────┤
│  Reserved/NVS (varies)           │
└─────────────────────────────────┘
```

### LittleFS Allocation

For web server, allocate:

```cpp
#define LITTLEFS_PARTITION_NAME "littlefs"
#define LITTLEFS_SIZE 0x0C000000  // ~12MB
#define LITTLEFS_PAGE_SIZE 256
#define LITTLEFS_BLOCK_SIZE 4096
```

**Available Space:**
- Web files: ~140KB (out of 12MB) ✅
- Configuration: ~2KB ✅
- Temporary (OTA): ~400KB ✅
- **Total used: ~550KB / 12000MB = 4.6% usage** ✅

---

## File Structure

### Directory Layout on LittleFS

```
/littlefs
├── /data
│   ├── index.html         (25 KB) - Dashboard
│   ├── config.html        (18 KB) - Configuration
│   ├── nodes.html         (16 KB) - Node monitor
│   ├── logs.html          (14 KB) - Logs viewer
│   ├── style.css          (35 KB) - Styling
│   └── app.js             (32 KB) - JavaScript application
│
├── /config
│   └── config.json        (2 KB) - System configuration
│
└── /logs (optional)
    └── system.log         (varies) - Log file (if implemented)
```

### File Sizes Reference

```
index.html          25,234 bytes  (gzip: ~7KB)
config.html         18,156 bytes  (gzip: ~5KB)
nodes.html          16,842 bytes  (gzip: ~4.5KB)
logs.html           14,678 bytes  (gzip: ~4KB)
style.css           35,421 bytes  (gzip: ~8KB)
app.js              32,546 bytes  (gzip: ~9KB)
config.json         ~2,000 bytes  (plain)
────────────────────────────────
TOTAL              144,877 bytes  (~43KB gzipped)
```

---

## Upload Methods

### Method 1: Arduino IDE (Easiest for Beginners)

**Prerequisites:**
- Arduino IDE 1.8.19+ or Arduino IDE 2.0+
- ESP32 board support installed
- LittleFS plugin installed

**Installation Steps:**

1. **Install LittleFS Tool Plugin**
   - Go to: https://github.com/earlephilhower/arduino-esp32fs-plugin/releases
   - Download latest `ESP32FS-X.X.X.zip`
   - Extract to `Arduino/tools/` directory:
     - Windows: `%USERPROFILE%\Documents\Arduino\tools\`
     - macOS: `~/Documents/Arduino/tools/`
     - Linux: `~/Arduino/tools/`
   - Create `tools/ESP32FS/tool/esp32fs.jar` structure if needed

2. **Prepare Files**
   - Create `esp32s3_central/data/` directory
   - Copy all web files into this directory:
     ```
     esp32s3_central/
     ├── esp32s3_central.ino
     └── data/
           ├── index.html
           ├── config.html
           ├── nodes.html
           ├── logs.html
           ├── style.css
           ├── app.js
           └── config/
                 └── config.json
     ```

3. **Upload from IDE**
   - Open `esp32s3_central.ino` in Arduino IDE
   - Select board: `ESP32-S3 Dev Module`
   - Go to: **Tools → Manage LittleFS → Upload LittleFS**
   - Wait for completion (2-5 minutes)
   - Watch Serial Monitor for: `[LittleFS] SPIFFS formatted successfully!`

**Pros:**
- ✅ Simple, integrated interface
- ✅ No command line required
- ✅ Automatic formatting

**Cons:**
- ❌ Slower than CLI methods
- ❌ Requires full Arduino IDE
- ❌ Limited feedback during upload

### Method 2: Arduino CLI (Recommended for Automation)

**Prerequisites:**
- arduino-cli 0.20.0+
- Python 3 (for esptool)
- LittleFS image creator

**Installation Steps:**

1. **Create LittleFS Image**
   ```bash
   # Windows
   mkdir build
   cd build

   # Using arduino-cli's built-in tools
   arduino-cli compile --fqbn esp32:esp32:esp32s3 ../esp32s3_central \
       --build-property build.littlefs_size=0x0C000000
   ```

2. **Create Binary Image**
   ```bash
   # Using mkspiffs (if installed)
   mkspiffs -c ..\esp32s3_central\data -b 4096 -p 256 -s 0x0C000000 \
       littlefs.bin
   ```

3. **Upload Using esptool**
   ```bash
   esptool.py --chip esp32s3 --port COM3 --baud 460800 \
       write_flash -z 0x400000 littlefs.bin
   ```

**Pros:**
- ✅ Fast and scriptable
- ✅ Full automation capability
- ✅ Can integrate into CI/CD

**Cons:**
- ❌ More complex setup
- ❌ Requires Python and multiple tools

### Method 3: PlatformIO (Modern Development)

**Prerequisites:**
- VSCode with PlatformIO extension
- platformio.ini configured for ESP32-S3

**Configuration (platformio.ini):**

```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

# LittleFS configuration
board_build.filesystem = littlefs
board_build.littlefs_pagesize = 256

# Define custom upload command
custom_upload_fs = yes
```

**Upload Command:**

```bash
# In VSCode Terminal
pio run -e esp32s3 --target uploadfs

# Or from command line
platformio run -e esp32s3 --target uploadfs
```

**File Structure for PlatformIO:**

```
project/
├── platformio.ini
├── include/
├── src/
│   └── esp32s3_central.ino
├── data/
│   ├── index.html
│   ├── config.html
│   ├── nodes.html
│   ├── logs.html
│   ├── style.css
│   ├── app.js
│   └── config/
│       └── config.json
└── lib/
```

**Pros:**
- ✅ Modern, integrated approach
- ✅ Automatic file management
- ✅ Better IDE integration in VSCode

**Cons:**
- ❌ Requires VSCode + PlatformIO extension
- ❌ Small learning curve

### Method 4: Direct esptool.py (Advanced)

**Prerequisites:**
- esptool.py installed: `pip install esptool`
- Pre-made LittleFS binary image

**Steps:**

1. **Create LittleFS binary** (using any image creator)
2. **Upload binary:**
   ```bash
   esptool.py --chip esp32s3 \
       --port /dev/ttyUSB0 \
       --baud 460800 \
       write_flash \
       -z 0x400000 littlefs.bin
   ```

**Output:**
```
esptool.py v3.3.2
Serial port /dev/ttyUSB0
Connecting...
Chip is ESP32-S3 (revision 0)
Features: WiFi, BLE, Embedded Flash, Embedded PSRAM
Crystal is 40MHz
MAC: XX:XX:XX:XX:XX:XX
Uploading stub...
Running stub...
Stub running...
Changing baud rate to 460800
Writing 0x0c000000 bytes to file offset 0x400000...
```

**Pros:**
- ✅ Direct control
- ✅ Scriptable
- ✅ Works on all platforms

**Cons:**
- ❌ Requires manual binary creation
- ❌ Most error-prone
- ❌ Manual address calculation needed

### Method 5: Web-Based Upload (Future Enhancement)

Once web server is running, you can implement a file upload endpoint:

```cpp
// In web_server.cpp
server.on("/api/upload-file", HTTP_POST, [](AsyncWebServerRequest *request){
    // Handle file upload
    // Save to LittleFS
}, [](AsyncWebServerRequest *request, const String& filename,
     size_t index, uint8_t *data, size_t len, bool final) {
    // Handle file chunks
    if (index == 0) {
        request->_tempFile = LittleFS.open("/" + filename, "w");
    }
    request->_tempFile.write(data, len);
    if (final) {
        request->_tempFile.close();
        request->send(200, "text/plain", "File uploaded");
    }
});
```

---

## Configuration Files

### config.json Structure

Location: `/config/config.json`

```json
{
  "system": {
    "hostname": "weatherstation",
    "first_boot": false,
    "version": "1.0.0"
  },

  "wifi": {
    "ssid": "YourNetworkSSID",
    "password": "YourPassword",
    "use_static_ip": false,
    "static_ip": "192.168.1.100",
    "gateway": "192.168.1.1",
    "subnet": "255.255.255.0",
    "dns": "8.8.8.8"
  },

  "api": {
    "openweathermap_key": "your_api_key_here",
    "tomorrow_io_key": "your_api_key_here",
    "latitude": 44.4268,
    "longitude": 26.1025
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

  "display": {
    "brightness": 80,
    "theme": "dark",
    "timeout_minutes": 30
  },

  "logging": {
    "enabled": true,
    "level": "INFO",
    "interval_minutes": 5
  },

  "auth": {
    "username": "admin",
    "password_hash": "8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918"
  }
}
```

### Default Configuration Creation

If `config.json` doesn't exist, create defaults:

```cpp
// In config_manager.cpp
bool ConfigManager::createDefaultConfig() {
    JsonDocument doc;

    // Default values
    doc["system"]["hostname"] = "weatherstation";
    doc["system"]["first_boot"] = true;

    doc["wifi"]["ssid"] = "YOUR_SSID";
    doc["wifi"]["password"] = "YOUR_PASSWORD";
    doc["wifi"]["use_static_ip"] = false;

    doc["api"]["openweathermap_key"] = "";
    doc["api"]["tomorrow_io_key"] = "";
    doc["api"]["latitude"] = 0.0;
    doc["api"]["longitude"] = 0.0;

    doc["display"]["brightness"] = 100;
    doc["display"]["timeout_minutes"] = 30;

    return saveConfig(doc);
}
```

---

## Code Implementation

### LittleFS Initialization in C++

```cpp
#include <LittleFS.h>

// Global LittleFS mount point
#define LITTLEFS_FORMAT_IF_MOUNT_FAILED true

bool initLittleFS() {
    Serial.println("[LittleFS] Mounting...");

    if (!LittleFS.begin(LITTLEFS_FORMAT_IF_MOUNT_FAILED)) {
        Serial.println("[ERROR] LittleFS mount failed!");
        return false;
    }

    // Print filesystem info
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    float percentUsed = (float)usedBytes / totalBytes * 100.0;

    Serial.printf("[LittleFS] Total: %lu bytes\n", totalBytes);
    Serial.printf("[LittleFS] Used: %lu bytes (%.1f%%)\n", usedBytes, percentUsed);
    Serial.printf("[LittleFS] Free: %lu bytes\n", totalBytes - usedBytes);

    // Warn if usage too high
    if (percentUsed > 80.0) {
        Serial.println("[WARN] LittleFS usage >80%!");
    }

    // List all files
    listLittleFSFiles("/");

    return true;
}

// Recursive file listing
void listLittleFSFiles(const char *dirname) {
    Serial.printf("[LittleFS] Files in %s:\n", dirname);

    File dir = LittleFS.open(dirname);
    if (!dir) {
        Serial.println("[ERROR] Failed to open directory");
        return;
    }

    File file = dir.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.printf("  [DIR]  %s/\n", file.name());
        } else {
            Serial.printf("  [FILE] %s (%lu bytes)\n", file.name(), file.size());
        }
        file = dir.openNextFile();
    }
}
```

### File Read/Write Operations

```cpp
// Read file as string
String readFileAsString(const char *filename) {
    File file = LittleFS.open(filename, "r");
    if (!file) {
        Serial.printf("[ERROR] File not found: %s\n", filename);
        return "";
    }

    String content;
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();

    return content;
}

// Write string to file
bool writeFileAsString(const char *filename, const String &content) {
    File file = LittleFS.open(filename, "w");
    if (!file) {
        Serial.printf("[ERROR] Failed to create: %s\n", filename);
        return false;
    }

    size_t written = file.print(content);
    file.close();

    if (written != content.length()) {
        Serial.printf("[ERROR] Write mismatch: %zu vs %zu bytes\n",
                     written, content.length());
        return false;
    }

    return true;
}

// Check file exists
bool fileExists(const char *filename) {
    return LittleFS.exists(filename);
}

// Delete file
bool deleteFile(const char *filename) {
    if (!fileExists(filename)) {
        return false;
    }
    return LittleFS.remove(filename);
}

// Get file size
size_t getFileSize(const char *filename) {
    File file = LittleFS.open(filename, "r");
    if (!file) return 0;

    size_t size = file.size();
    file.close();
    return size;
}
```

---

## Performance & Optimization

### Read Performance

```
Typical read speeds:
- Single byte: ~100 µs
- 1 KB: ~500 µs
- 10 KB: ~5 ms
- 100 KB: ~50 ms

Bottleneck: Flash memory speed, not LittleFS algorithm
```

### Write Performance

```
Typical write speeds:
- Single byte: ~200 µs
- 1 KB: ~1 ms
- 10 KB: ~10 ms
- 100 KB: ~100 ms

Note: Wear leveling adds overhead
```

### Optimization Tips

1. **Minimize file access frequency**
   ```cpp
   // BAD: Read config every loop iteration
   if (needUpdate) {
       String ssid = readFileAsString("/config.json");
       // ...
   }

   // GOOD: Cache in memory
   String cachedSSID = "";
   if (needUpdate) {
       if (cachedSSID.empty()) {
           cachedSSID = readFileAsString("/config.json");
       }
   }
   ```

2. **Use appropriate file modes**
   ```cpp
   // For multiple writes, keep file open
   File f = LittleFS.open("/data.log", "a");
   f.println("log 1");
   f.println("log 2");
   f.println("log 3");
   f.close();  // Close only when done
   ```

3. **Batch operations**
   ```cpp
   // Create JSON in memory, write once
   JsonDocument doc;
   doc["sensor1"] = 25.5;
   doc["sensor2"] = 60.2;
   String json;
   serializeJson(doc, json);
   writeFileAsString("/data.json", json);  // Single write
   ```

4. **Compression for large files** (if needed)
   ```cpp
   // Use gzip compression for files >50KB
   // Trade-off: CPU usage vs. storage savings
   ```

---

## Maintenance

### Periodic Tasks

**Daily:**
- Monitor LittleFS usage via web interface
- Check for I/O errors in logs

**Weekly:**
- Verify all web files accessible
- Test OTA update mechanism
- Backup config.json

**Monthly:**
- Analyze fragmentation (if tools available)
- Defragmentation (if supported)

### Checking Filesystem Health

```cpp
void checkLittleFSHealth() {
    size_t total = LittleFS.totalBytes();
    size_t used = LittleFS.usedBytes();
    size_t free = total - used;

    Serial.printf("[Health] Total: %lu, Used: %lu, Free: %lu\n",
                 total, used, free);

    if (free < 500000) {  // < 500KB free
        Serial.println("[WARN] Low free space!");
    }

    if (used > total * 0.9) {  // > 90% used
        Serial.println("[WARN] >90% capacity!");
    }
}

// Call periodically (e.g., daily)
if (checkHealthTimer.check()) {
    checkLittleFSHealth();
}
```

### Backup Configuration

```cpp
bool backupConfig() {
    String config = readFileAsString("/config/config.json");
    if (config.empty()) return false;

    // Create backup with timestamp
    time_t now = time(nullptr);
    char filename[50];
    snprintf(filename, sizeof(filename), "/config/backup_%ld.json", now);

    return writeFileAsString(filename, config);
}

// Keep only last 5 backups
bool cleanOldBackups() {
    File dir = LittleFS.open("/config");
    std::vector<String> backups;

    File file = dir.openNextFile();
    while (file) {
        String name = file.name();
        if (name.startsWith("backup_")) {
            backups.push_back(name);
        }
        file = dir.openNextFile();
    }

    // Sort and delete oldest if >5
    if (backups.size() > 5) {
        std::sort(backups.begin(), backups.end());
        for (int i = 0; i < backups.size() - 5; i++) {
            LittleFS.remove("/config/" + backups[i]);
        }
    }

    return true;
}
```

---

## Troubleshooting

### Problem: LittleFS won't mount

**Symptoms:**
- `LittleFS mount failed!` in Serial Monitor
- Web server doesn't start
- Files cannot be read

**Solutions:**

1. **Format partition:**
   ```cpp
   bool formatLittleFS() {
       Serial.println("[LittleFS] Formatting...");
       if (!LittleFS.format()) {
           Serial.println("[ERROR] Format failed!");
           return false;
       }
       Serial.println("[OK] Format successful");
       return true;
   }

   // Call in setup if needed:
   if (!LittleFS.begin()) {
       formatLittleFS();
       LittleFS.begin();
   }
   ```

2. **Verify partition table:**
   - Check `menuconfig` or partition definition
   - Ensure LittleFS partition allocated

3. **Check flash integrity:**
   ```bash
   esptool.py --chip esp32s3 read_flash 0x0 0x1000000 flash_dump.bin
   esptool.py --chip esp32s3 verify_flash flash_dump.bin
   ```

### Problem: Files can't be read

**Symptoms:**
- `File not found` errors
- Web pages return 404
- API endpoints fail

**Solutions:**

1. **Verify file upload:**
   ```cpp
   if (!LittleFS.exists("/data/index.html")) {
       Serial.println("[ERROR] index.html not found!");
       Serial.println("[INFO] Files in root:");
       listLittleFSFiles("/");
   }
   ```

2. **Check file paths:**
   - Paths are case-sensitive: `/data/Index.html` ≠ `/data/index.html`
   - Use forward slashes: `/data/file.txt`
   - Full absolute paths required

3. **Verify upload completed:**
   - Check Serial Monitor for success message
   - Reupload if download interrupted

### Problem: Out of space

**Symptoms:**
- `Write failed` errors
- Cannot save configuration
- OTA uploads fail

**Solutions:**

1. **Check usage:**
   ```cpp
   Serial.printf("[LittleFS] Used: %lu / %lu bytes\n",
                LittleFS.usedBytes(), LittleFS.totalBytes());
   ```

2. **Delete unnecessary files:**
   ```cpp
   if (LittleFS.exists("/logs/old.log")) {
       LittleFS.remove("/logs/old.log");
   }
   ```

3. **Increase allocation** (rebuild firmware):
   - Modify partition table
   - Recompile and flash

### Problem: Corrupted files

**Symptoms:**
- Garbled content
- CRC errors
- Web pages render incorrectly

**Solutions:**

1. **Verify file integrity:**
   ```cpp
   void verifyFileIntegrity(const char *filename) {
       File file = LittleFS.open(filename, "r");
       if (!file) return;

       uint32_t crc = 0;
       while (file.available()) {
           crc = crc32_be(crc, (uint8_t)file.read());
       }
       file.close();

       Serial.printf("[Verify] %s CRC: 0x%08lx\n", filename, crc);
   }
   ```

2. **Rebuild from backup:**
   - Reupload web files
   - Restore config.json from backup

3. **Full filesystem rebuild:**
   - Format partition
   - Reupload all files

---

## Backup & Recovery

### Automated Backup System

```cpp
class BackupManager {
  private:
    static const uint8_t MAX_BACKUPS = 5;

  public:
    // Backup important files
    bool backupSystem() {
        time_t now = time(nullptr);
        char backupDir[50];
        snprintf(backupDir, sizeof(backupDir), "/backups/%ld", now);

        // Create backup directory
        if (!LittleFS.mkdir(backupDir)) {
            Serial.println("[Backup] Directory creation failed");
            return false;
        }

        // Backup config
        String config = readFileAsString("/config/config.json");
        String backupPath = String(backupDir) + "/config.json";
        if (!writeFileAsString(backupPath.c_str(), config)) {
            Serial.println("[Backup] Config backup failed");
            return false;
        }

        Serial.println("[Backup] Backup created: " + String(backupDir));

        // Clean old backups
        cleanOldBackups();

        return true;
    }

    // Restore from backup
    bool restoreFromBackup(const char *backupDir) {
        String config = readFileAsString((String(backupDir) + "/config.json").c_str());
        if (config.empty()) {
            Serial.println("[Restore] Backup file not found");
            return false;
        }

        if (!writeFileAsString("/config/config.json", config)) {
            Serial.println("[Restore] Restore failed");
            return false;
        }

        Serial.println("[Restore] Restored from: " + String(backupDir));
        return true;
    }

  private:
    void cleanOldBackups() {
        File dir = LittleFS.open("/backups");
        std::vector<String> backups;

        File file = dir.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                backups.push_back(file.name());
            }
            file = dir.openNextFile();
        }

        std::sort(backups.rbegin(), backups.rend());

        for (int i = MAX_BACKUPS; i < backups.size(); i++) {
            String path = "/backups/" + backups[i];
            deleteDirectory(path.c_str());
        }
    }

    bool deleteDirectory(const char *path) {
        File dir = LittleFS.open(path);
        File file = dir.openNextFile();

        while (file) {
            if (!file.isDirectory()) {
                LittleFS.remove(file.name());
            }
            file = dir.openNextFile();
        }

        return LittleFS.rmdir(path);
    }
};
```

### Recovery from Web Interface

```cpp
// Add to web_server.cpp
server.on("/api/backup", HTTP_POST, [](AsyncWebServerRequest *request) {
    BackupManager backup;
    if (backup.backupSystem()) {
        request->send(200, "application/json",
                     "{\"status\": \"Backup created\"}");
    } else {
        request->send(500, "application/json",
                     "{\"error\": \"Backup failed\"}");
    }
});

server.on("/api/restore", HTTP_POST, [](AsyncWebServerRequest *request) {
    String backupDir = request->getParam("backup")->value();
    BackupManager backup;
    if (backup.restoreFromBackup(backupDir.c_str())) {
        request->send(200, "application/json",
                     "{\"status\": \"Restored successfully\"}");
    } else {
        request->send(500, "application/json",
                     "{\"error\": \"Restore failed\"}");
    }
});
```

---

## Quick Reference

### Essential Commands

```bash
# List files in LittleFS
esptool.py --chip esp32s3 spiffs_list_partition /

# Upload files with Arduino CLI
arduino-cli compile --fqbn esp32:esp32:esp32s3 --build-property build.littlefs_size=0x0C000000 ./project

# Upload with esptool
esptool.py write_flash -z 0x400000 littlefs.bin

# Check LittleFS usage
Serial Monitor: Shows in initialization
Web Interface: Dashboard → System Status
```

### File Size Limits

```
ESP32-S3 with 16MB flash:
- Total LittleFS: ~12MB
- Single file max: Limited by total size
- Recommended max file: <1MB (for reliability)
- Practical limit: 10MB usable (90% of 12MB)
```

### Performance Benchmarks

```
Operation               Time
──────────────────────────────
Read 1 byte            ~100 µs
Read 1 KB              ~500 µs
Read 10 KB             ~5 ms
Write 1 KB             ~1 ms
Write 10 KB            ~10 ms
Format 1MB partition   ~1-2 seconds
```

---

## Best Practices

1. ✅ **Always use absolute paths** starting with `/`
2. ✅ **Check file existence** before reading
3. ✅ **Close files properly** to prevent corruption
4. ✅ **Monitor free space** regularly
5. ✅ **Create backups** of critical config
6. ✅ **Use JSON** for configuration files
7. ✅ **Handle corrupted files** gracefully
8. ✅ **Test recovery procedures** periodically
9. ✅ **Document file structure** for team
10. ✅ **Version your configs** with timestamps

---

**Last Updated:** 2025-11-22
**Status:** Complete and production-ready

