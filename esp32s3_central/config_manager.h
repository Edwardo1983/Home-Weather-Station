/**
 * @file config_manager.h
 * @brief Configuration management using LittleFS
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

/**
 * Manages system configuration stored in LittleFS
 */
class ConfigManager {
public:
    ConfigManager(const char* configPath = "/config.json");

    /**
     * Load configuration from LittleFS
     */
    bool loadConfig();

    /**
     * Save configuration to LittleFS
     */
    bool saveConfig();

    /**
     * Get WiFi SSID
     */
    String getWiFiSSID() const { return wifiSSID; }

    /**
     * Get WiFi password
     */
    String getWiFiPassword() const { return wifiPassword; }

    /**
     * Get API key (OpenWeatherMap)
     */
    String getAPIKeyOWM() const { return apiKeyOWM; }

    /**
     * Get API key (Tomorrow.io)
     */
    String getAPIKeyTomorrow() const { return apiKeyTomorrow; }

    /**
     * Get latitude
     */
    float getLatitude() const { return latitude; }

    /**
     * Get longitude
     */
    float getLongitude() const { return longitude; }

    /**
     * Set WiFi credentials
     */
    void setWiFiCredentials(const String& ssid, const String& password);

    /**
     * Set API keys
     */
    void setAPIKeys(const String& owm, const String& tomorrow);

    /**
     * Set location
     */
    void setLocation(float lat, float lon);

    /**
     * Set display settings
     */
    void setDisplaySettings(uint8_t brightness, uint8_t timeout);

    /**
     * Get display brightness (0-100)
     */
    uint8_t getDisplayBrightness() const { return displayBrightness; }

    /**
     * Get display timeout (minutes)
     */
    uint8_t getDisplayTimeout() const { return displayTimeout; }

    /**
     * Factory reset
     */
    bool factoryReset();

    /**
     * Export configuration as JSON string
     */
    String exportConfig();

    /**
     * Import configuration from JSON string
     */
    bool importConfig(const String& json);

    /**
     * Get raw JSON document
     */
    JsonDocument& getDocument() { return configDoc; }

private:
    String configPath;
    DynamicJsonDocument configDoc;

    // Cached values
    String wifiSSID;
    String wifiPassword;
    String apiKeyOWM;
    String apiKeyTomorrow;
    float latitude;
    float longitude;
    uint8_t displayBrightness;
    uint8_t displayTimeout;

    /**
     * Create default configuration
     */
    void createDefaultConfig();

    /**
     * Parse configuration document
     */
    void parseConfig();
};

#endif // CONFIG_MANAGER_H
