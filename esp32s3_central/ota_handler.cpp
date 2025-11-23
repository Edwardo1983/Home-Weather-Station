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

    // Write data chunk
    if (!writeData(data, len)) {
        return false;
    }

    progress = (index + len) * 100 / totalSize;

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
    // Basic validation - could be enhanced with signature checks
    if (len < 16) return false;

    // Check ESP32 image header
    if (data[0] != 0xe9) {
        Serial.println(F("[WARNING] Invalid ESP32 image header"));
        return false;
    }

    return true;
}
