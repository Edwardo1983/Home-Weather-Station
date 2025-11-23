/**
 * @file web_server.h
 * @brief AsyncWebServer management for ESP32-S3 central node
 * @details Provides HTTP/WebSocket interface for admin control
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>

// Forward declarations
class WebSocketHandler;
class ConfigManager;
class OTAHandler;

/**
 * Web server interface for system control and monitoring
 */
class WebServer {
public:
    WebServer(uint16_t port = 80, uint16_t wsPort = 81);
    ~WebServer();

    /**
     * Initialize web server
     */
    bool begin();

    /**
     * Stop web server
     */
    void end();

    /**
     * Handle incoming WebSocket messages
     */
    void handleWebSocketMessage(const String& message);

    /**
     * Broadcast data to all connected WebSocket clients
     */
    void broadcastWebSocketData(const String& jsonData);

    /**
     * Check if web server is running
     */
    bool isRunning() const { return running; }

    /**
     * Get connected WebSocket client count
     */
    uint8_t getWebSocketClientCount() const { return wsClientCount; }

    /**
     * Set system state references (for API calls)
     */
    void setSensorManager(class SensorManager* mgr) { sensorMgr = mgr; }
    void setESPNowReceiver(class ESPNowReceiver* rcv) { espnowRcv = rcv; }
    void setWeatherAPI(class WeatherAPI* api) { weatherApi = api; }
    void setConfigManager(class ConfigManager* cfg) { configMgr = cfg; }
    void setOTAHandler(class OTAHandler* ota) { otaHandler = ota; }

private:
    AsyncWebServer* server;
    AsyncWebSocket* ws;

    uint16_t httpPort;
    uint16_t wsPort;
    bool running;
    uint8_t wsClientCount;

    // Pointers to system modules
    class SensorManager* sensorMgr;
    class ESPNowReceiver* espnowRcv;
    class WeatherAPI* weatherApi;
    class ConfigManager* configMgr;
    class OTAHandler* otaHandler;

    // =================================================================
    // HTTP Request Handlers
    // =================================================================

    /**
     * Serve static files from LittleFS
     */
    void setupStaticFileServing();

    /**
     * Setup API endpoints
     */
    void setupAPIEndpoints();

    /**
     * Setup WebSocket handler
     */
    void setupWebSocket();

    /**
     * Setup mDNS
     */
    void setupMDNS();

    // =================================================================
    // API Endpoint Callbacks
    // =================================================================

    /**
     * GET /api/status - System status
     */
    void handleAPIStatus(AsyncWebServerRequest* request);

    /**
     * GET /api/sensors - Current sensor readings
     */
    void handleAPISensors(AsyncWebServerRequest* request);

    /**
     * GET /api/nodes - Remote node status
     */
    void handleAPINodes(AsyncWebServerRequest* request);

    /**
     * GET /api/weather - Weather API data
     */
    void handleAPIWeather(AsyncWebServerRequest* request);

    /**
     * GET /api/config - Get configuration (without sensitive data)
     */
    void handleAPIConfig(AsyncWebServerRequest* request);

    /**
     * POST /api/config - Save configuration
     */
    void handleAPIConfigSave(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total);

    /**
     * GET /api/logs - Get system logs
     */
    void handleAPILogs(AsyncWebServerRequest* request);

    /**
     * POST /api/system/restart - Restart system
     */
    void handleAPIRestart(AsyncWebServerRequest* request);

    /**
     * POST /api/wifi/scan - Scan WiFi networks
     */
    void handleAPIWiFiScan(AsyncWebServerRequest* request);

    /**
     * POST /api/ota/upload - Upload firmware
     */
    void handleAPIOTAUpload(AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final);

    /**
     * WebSocket event handler
     */
    void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

    // =================================================================
    // Helper Functions
    // =================================================================

    /**
     * Check if request is authenticated (Basic Auth)
     */
    bool isAuthenticated(AsyncWebServerRequest* request);

    /**
     * Send JSON response
     */
    void sendJSON(AsyncWebServerRequest* request, JsonDocument& doc);

    /**
     * Get current system status as JSON
     */
    JsonDocument getSystemStatus();

    /**
     * Get sensor data as JSON
     */
    JsonDocument getSensorData();

    /**
     * Get node status as JSON
     */
    JsonDocument getNodeStatus();

    // Static wrapper functions for callbacks
    static void staticHandleAPIStatus(AsyncWebServerRequest* request) {
        // Would need static instance pointer - handled via lambda in implementation
    }
};

#endif // WEB_SERVER_H
