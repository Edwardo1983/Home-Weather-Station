/**
 * @file ota_handler.cpp
 * @brief OTA firmware update implementation
 */

#include "ota_handler.h"

OTAHandler::OTAHandler()
    : progress(0), updating(false), totalSize(0) {}

bool OTAHandler::begin() {
    Serial.println(F("[OTA] Handler initialized"));
    return true;
}

bool OTAHandler::handleFirmwareUpload(const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
    if (!updating && index == 0) {
        // CRITICAL: Validate firmware BEFORE starting update
        if (!validateFirmware(data, len)) {
            lastError = "Firmware validation failed";
            Serial.println(F("[ERROR] Invalid firmware - validation failed"));
            return false;
        }

        // Start of upload
        if (!startUpdate(0)) {  // Size will be determined during upload
            return false;
        }
    }

    if (!updating) {
        lastError = "OTA not started";
        Serial.println(F("[ERROR] OTA not started"));
        return false;
    }

    // CRITICAL: Validate firmware size doesn't exceed limits
    totalSize += len;
    if (totalSize > MAX_FIRMWARE_SIZE) {
        lastError = "Firmware too large";
        Serial.println(F("[ERROR] Firmware exceeds maximum size (4MB)"));
        Update.abort();
        updating = false;
        return false;
    }

    // Write data chunk
    if (!writeData(data, len)) {
        return false;
    }

    if (final) {
        // End of upload
        return finishUpdate();
    }

    return true;
}

bool OTAHandler::startUpdate(size_t size) {
    Serial.println(F("[OTA] Starting firmware update..."));

    // Begin update (size 0 = unknown, will be determined)
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
        lastError = "Update.begin() failed";
        Serial.print(F("[ERROR] "));
        Serial.println(lastError);
        return false;
    }

    updating = true;
    totalSize = size;
    progress = 0;

    Serial.println(F("[OK] OTA update started"));
    return true;
}

bool OTAHandler::writeData(uint8_t* data, size_t len) {
    size_t written = Update.write(data, len);

    if (written != len) {
        lastError = "Write failed";
        Serial.println(F("[ERROR] Firmware write failed"));
        Update.abort();
        updating = false;
        return false;
    }

    return true;
}

bool OTAHandler::finishUpdate() {
    if (!Update.end(true)) {
        lastError = "Update.end() failed";
        Serial.print(F("[ERROR] "));
        Serial.println(lastError);
        updating = false;
        return false;
    }

    updating = false;
    progress = 100;

    Serial.println(F("[OK] Firmware update completed successfully"));
    Serial.println(F("[INFO] System will restart in 2 seconds..."));

    delay(2000);
    ESP.restart();

    return true;
}

bool OTAHandler::validateFirmware(const uint8_t* data, size_t len) {
    // CRITICAL: Validate firmware file structure and magic bytes

    // Minimum firmware size check
    if (len < 512) {
        Serial.println(F("[ERROR] Firmware too small - minimum 512 bytes required"));
        return false;
    }

    // Check ESP32 magic byte (first byte of bootloader)
    if (data[0] != FIRMWARE_MAGIC) {
        Serial.print(F("[ERROR] Invalid firmware magic byte: 0x"));
        Serial.println(data[0], HEX);
        return false;
    }

    // Log firmware validation success
    Serial.print(F("[OK] Firmware validated: "));
    Serial.print(len);
    Serial.println(F(" bytes"));

    return true;
}
