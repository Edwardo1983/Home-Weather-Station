/**
 * @file ota_handler.h
 * @brief Over-The-Air firmware update handler
 */

#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <Arduino.h>
#include <Update.h>

/**
 * Handles OTA firmware updates
 */
class OTAHandler {
public:
    OTAHandler();

    /**
     * Initialize OTA handler
     */
    bool begin();

    /**
     * Handle incoming firmware data
     */
    bool handleFirmwareUpload(const String& filename, size_t index, uint8_t* data, size_t len, bool final);

    /**
     * Get current firmware version
     */
    String getFirmwareVersion() const { return "1.0.0"; }

    /**
     * Get OTA progress (0-100)
     */
    uint8_t getProgress() const { return progress; }

    /**
     * Check if OTA is in progress
     */
    bool isUpdating() const { return updating; }

    /**
     * Get last error message
     */
    String getLastError() const { return lastError; }

private:
    uint8_t progress;
    bool updating;
    String lastError;
    size_t totalSize;

    /**
     * Validate firmware file
     */
    bool validateFirmware(const uint8_t* data, size_t len);

    /**
     * Start firmware update
     */
    bool startUpdate(size_t size);

    /**
     * Write firmware data
     */
    bool writeData(uint8_t* data, size_t len);

    /**
     * Finish firmware update
     */
    bool finishUpdate();
};

#endif // OTA_HANDLER_H
