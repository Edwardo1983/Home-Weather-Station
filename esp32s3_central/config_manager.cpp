/**
 * @file config_manager.cpp
 * @brief Configuration manager implementation
 */

#include "config_manager.h"

ConfigManager::ConfigManager(const char* configPath)
    : configPath(configPath), configDoc(2048),
      latitude(44.4268), longitude(26.1025),
      displayBrightness(80), displayTimeout(10) {
    createDefaultConfig();
}

bool ConfigManager::loadConfig() {
    if (!LittleFS.begin()) {
        Serial.println(F("[ConfigManager] LittleFS mount failed"));
        return false;
    }

    if (!LittleFS.exists(configPath)) {
        Serial.println(F("[ConfigManager] Config file not found, creating default"));
        return saveConfig();
    }

    File file = LittleFS.open(configPath, "r");
    if (!file) {
        Serial.println(F("[ERROR] Failed to open config file"));
        return false;
    }

    DeserializationError error = deserializeJson(configDoc, file);
    file.close();

    if (error) {
        Serial.println(F("[ERROR] JSON parsing failed"));
        return false;
    }

    parseConfig();
    Serial.println(F("[OK] Configuration loaded from LittleFS"));
    return true;
}

bool ConfigManager::saveConfig() {
    File file = LittleFS.open(configPath, "w");
    if (!file) {
        Serial.println(F("[ERROR] Failed to create config file"));
        return false;
    }

    // Update document with current values
    configDoc["wifi"]["ssid"] = wifiSSID;
    configDoc["wifi"]["password"] = wifiPassword;
    configDoc["api"]["openweathermap_key"] = apiKeyOWM;
    configDoc["api"]["tomorrow_key"] = apiKeyTomorrow;
    configDoc["api"]["latitude"] = latitude;
    configDoc["api"]["longitude"] = longitude;
    configDoc["display"]["brightness"] = displayBrightness;
    configDoc["display"]["timeout_minutes"] = displayTimeout;

    serializeJson(configDoc, file);
    file.close();

    Serial.println(F("[OK] Configuration saved to LittleFS"));
    return true;
}

void ConfigManager::createDefaultConfig() {
    configDoc.clear();

    // WiFi
    configDoc["wifi"]["ssid"] = "YOUR_SSID";
    configDoc["wifi"]["password"] = "YOUR_PASSWORD";
    configDoc["wifi"]["static_ip"] = false;

    // API Keys
    configDoc["api"]["openweathermap_key"] = "";
    configDoc["api"]["tomorrow_key"] = "";
    configDoc["api"]["latitude"] = latitude;
    configDoc["api"]["longitude"] = longitude;

    // Display
    configDoc["display"]["brightness"] = displayBrightness;
    configDoc["display"]["timeout_minutes"] = displayTimeout;
    configDoc["display"]["theme"] = "dark";

    // Logging
    configDoc["logging"]["enabled"] = true;
    configDoc["logging"]["interval_minutes"] = 5;

    // System
    configDoc["system"]["hostname"] = "weatherstation";
    configDoc["system"]["first_boot"] = true;

    // Authentication
    configDoc["auth"]["username"] = "admin";
    configDoc["auth"]["password_hash"] = "";

    parseConfig();
}

void ConfigManager::parseConfig() {
    wifiSSID = configDoc["wifi"]["ssid"] | "";
    wifiPassword = configDoc["wifi"]["password"] | "";
    apiKeyOWM = configDoc["api"]["openweathermap_key"] | "";
    apiKeyTomorrow = configDoc["api"]["tomorrow_key"] | "";
    latitude = configDoc["api"]["latitude"] | 44.4268;
    longitude = configDoc["api"]["longitude"] | 26.1025;
    displayBrightness = configDoc["display"]["brightness"] | 80;
    displayTimeout = configDoc["display"]["timeout_minutes"] | 10;
}

void ConfigManager::setWiFiCredentials(const String& ssid, const String& password) {
    wifiSSID = ssid;
    wifiPassword = password;
    saveConfig();
    Serial.println(F("[ConfigManager] WiFi credentials updated"));
}

void ConfigManager::setAPIKeys(const String& owm, const String& tomorrow) {
    apiKeyOWM = owm;
    apiKeyTomorrow = tomorrow;
    saveConfig();
    Serial.println(F("[ConfigManager] API keys updated"));
}

void ConfigManager::setLocation(float lat, float lon) {
    latitude = lat;
    longitude = lon;
    saveConfig();
    Serial.println(F("[ConfigManager] Location updated"));
}

void ConfigManager::setDisplaySettings(uint8_t brightness, uint8_t timeout) {
    displayBrightness = brightness;
    displayTimeout = timeout;
    saveConfig();
    Serial.println(F("[ConfigManager] Display settings updated"));
}

bool ConfigManager::factoryReset() {
    if (LittleFS.remove(configPath)) {
        createDefaultConfig();
        saveConfig();
        Serial.println(F("[ConfigManager] Factory reset completed"));
        return true;
    }
    Serial.println(F("[ERROR] Factory reset failed"));
    return false;
}

String ConfigManager::exportConfig() {
    String json;
    serializeJson(configDoc, json);
    return json;
}

bool ConfigManager::importConfig(const String& json) {
    DynamicJsonDocument tempDoc(2048);
    DeserializationError error = deserializeJson(tempDoc, json);

    if (error) {
        Serial.println(F("[ERROR] Invalid JSON import"));
        return false;
    }

    configDoc = tempDoc;
    parseConfig();
    return saveConfig();
}
