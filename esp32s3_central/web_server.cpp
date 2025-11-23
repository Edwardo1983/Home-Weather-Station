/**
 * @file web_server.cpp
 * @brief Web server implementation
 */

#include "web_server.h"
#include "sensor_manager.h"
#include "espnow_receiver.h"
#include "weather_api.h"
#include "config_manager.h"
#include "ota_handler.h"
#include <LittleFS.h>

// Static instance for lambda callbacks
static WebServer* webServerInstance = nullptr;

// CRITICAL: Authentication token (should be from config in production)
const char* WebServer::ADMIN_TOKEN = "weather_station_2025";

WebServer::WebServer(uint16_t port, uint16_t wsPort)
    : server(nullptr), ws(nullptr), httpPort(port), wsPort(wsPort),
      running(false), wsClientCount(0),
      sensorMgr(nullptr), espnowRcv(nullptr), weatherApi(nullptr),
      configMgr(nullptr), otaHandler(nullptr) {
    webServerInstance = this;
}

WebServer::~WebServer() {
    end();
}

bool WebServer::begin() {
    if (running) return true;

    Serial.println(F("[WebServer] Starting async web server..."));

    // Create AsyncWebServer instance
    server = new AsyncWebServer(httpPort);
    ws = new AsyncWebSocket("/ws");

    if (!server || !ws) {
        Serial.println(F("[ERROR] Failed to allocate web server"));
        return false;
    }

    // Setup mDNS
    setupMDNS();

    // Setup static file serving
    setupStaticFileServing();

    // Setup WebSocket
    setupWebSocket();

    // Setup API endpoints
    setupAPIEndpoints();

    // Handle 404
    server->onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "application/json", "{\"error\":\"Not Found\"}");
    });

    server->begin();
    running = true;

    Serial.println(F("[OK] Web server started on port 80"));
    Serial.println(F("[OK] WebSocket on port 81"));
    Serial.println(F("[OK] Access at: http://weatherstation.local or http://"));
    Serial.print(WiFi.localIP());
    Serial.println();

    return true;
}

void WebServer::end() {
    if (!running) return;

    if (ws) {
        ws->closeAll();
        delete ws;
        ws = nullptr;
    }

    if (server) {
        server->end();
        delete server;
        server = nullptr;
    }

    running = false;
    Serial.println(F("[WebServer] Stopped"));
}

void WebServer::setupMDNS() {
    if (!MDNS.begin("weatherstation")) {
        Serial.println(F("[WARNING] mDNS begin failed"));
    } else {
        MDNS.addService("http", "tcp", httpPort);
        Serial.println(F("[OK] mDNS: weatherstation.local"));
    }
}

void WebServer::setupStaticFileServing() {
    // Mount LittleFS
    if (!LittleFS.begin()) {
        Serial.println(F("[ERROR] LittleFS mount failed"));
        return;
    }

    Serial.println(F("[OK] LittleFS mounted"));

    // Serve static files
    server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    Serial.println(F("[OK] Static files serving from LittleFS"));
}

void WebServer::setupWebSocket() {
    ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
        this->onWebSocketEvent(server, client, type, arg, data, len);
    });

    server->addHandler(ws);
    Serial.println(F("[OK] WebSocket handler registered"));
}

void WebServer::setupAPIEndpoints() {
    // System Status
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        DynamicJsonDocument doc(512);
        doc["uptime"] = millis();
        doc["heap"] = esp_get_free_heap_size();
        doc["wifi_signal"] = WiFi.RSSI();
        doc["temperature"] = temperatureRead();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // Sensor Data
    server->on("/api/sensors", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!sensorMgr) {
            request->send(503, "application/json", "{\"error\":\"Sensor manager not initialized\"}");
            return;
        }
        handleAPISensors(request);
    });

    // Node Status
    server->on("/api/nodes", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!espnowRcv) {
            request->send(503, "application/json", "{\"error\":\"ESP-NOW receiver not initialized\"}");
            return;
        }
        handleAPINodes(request);
    });

    // WiFi Scan
    server->on("/api/wifi/scan", HTTP_POST, [this](AsyncWebServerRequest* request) {
        int networksFound = WiFi.scanNetworks(false, false);
        DynamicJsonDocument doc(1024);
        doc["count"] = networksFound;

        JsonArray networks = doc.createNestedArray("networks");
        for (int i = 0; i < networksFound; i++) {
            networks.add(WiFi.SSID(i));
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // System Restart
    server->on("/api/system/restart", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated(request)) {
            request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
            return;
        }
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Restarting...\"}");
        delay(1000);
        ESP.restart();
    });

    // Configuration Get
    server->on("/api/config/get", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!configMgr) {
            request->send(503, "application/json", "{\"error\":\"Config manager not initialized\"}");
            return;
        }
        handleAPIConfig(request);
    });

    // Configuration Save (WiFi)
    server->on("/api/config/wifi", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated(request)) {
            request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
            return;
        }
        request->onData([this](AsyncWebServerRequest* r, uint8_t* data, size_t len, size_t index, size_t total) {
            handleAPIConfigSave(r, data, len, index, total);
        });
    });

    // OTA Upload endpoint
    server->on("/api/ota/upload", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!isAuthenticated(request)) {
            request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
            return;
        }
        request->send(200);
    }, [this](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
        if (otaHandler) {
            otaHandler->handleFirmwareUpload(filename, index, data, len, final);
        }
    });

    // Logs endpoint
    server->on("/api/logs", HTTP_GET, [](AsyncWebServerRequest* request) {
        // Return recent log entries (implementation depends on logging system)
        DynamicJsonDocument doc(2048);
        JsonArray logs = doc.createNestedArray("logs");

        // Placeholder - would fetch from circular buffer
        request->send(200, "application/json", "{\"logs\":[]}");
    });

    Serial.println(F("[OK] API endpoints registered"));
}

void WebServer::onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        wsClientCount++;
        Serial.print(F("[WebSocket] Client connected, total: "));
        Serial.println(wsClientCount);

        // Send initial data
        DynamicJsonDocument doc(256);
        doc["type"] = "connection_established";
        doc["message"] = "Connected to Weather Station";

        String response;
        serializeJson(doc, response);
        client->text(response);

    } else if (type == WS_EVT_DISCONNECT) {
        wsClientCount--;
        Serial.print(F("[WebSocket] Client disconnected, total: "));
        Serial.println(wsClientCount);

    } else if (type == WS_EVT_DATA) {
        // Parse incoming message
        // CRITICAL: Use String directly instead of malloc for exception safety
        if (len > 0) {
            String payload((char*)data, len);
            if (!payload.isEmpty()) {
                handleWebSocketMessage(payload);
            }
        }
    }
}

void WebServer::broadcastWebSocketData(const String& jsonData) {
    if (!ws || !running) return;

    ws->textAll(jsonData);
}

void WebServer::handleWebSocketMessage(const String& message) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.println(F("[WebSocket] JSON parse error"));
        return;
    }

    const char* type = doc["type"] | "";

    if (strcmp(type, "request_sensor_data") == 0) {
        // Send current sensor data
        if (sensorMgr) {
            DynamicJsonDocument response(512);
            response["type"] = "sensor_update";
            // Would populate with actual sensor data
            String jsonStr;
            serializeJson(response, jsonStr);
            broadcastWebSocketData(jsonStr);
        }
    } else if (strcmp(type, "request_system_status") == 0) {
        // Send system status
        DynamicJsonDocument response(256);
        response["type"] = "system_status";
        response["uptime"] = millis();
        response["heap"] = esp_get_free_heap_size();

        String jsonStr;
        serializeJson(response, jsonStr);
        broadcastWebSocketData(jsonStr);
    }
}

void WebServer::handleAPISensors(AsyncWebServerRequest* request) {
    // Build sensor data response
    DynamicJsonDocument doc(1024);

    if (sensorMgr) {
        SensorData data = sensorMgr->getLastData();
        doc["indoor_main"]["temperature"] = data.temperature;
        doc["indoor_main"]["humidity"] = data.humidity;
        doc["indoor_main"]["pressure"] = data.pressure;
        doc["indoor_main"]["iaq"] = data.iaq;
    }

    if (espnowRcv) {
        ESPNowData interiorData = espnowRcv->getInteriorData();
        doc["indoor_secondary"]["temperature"] = interiorData.temperature;
        doc["indoor_secondary"]["humidity"] = interiorData.humidity;

        ESPNowData exteriorData = espnowRcv->getExteriorData();
        doc["outdoor"]["temperature"] = exteriorData.temperature;
        doc["outdoor"]["humidity"] = exteriorData.humidity;
        doc["outdoor"]["pressure"] = exteriorData.pressure;
        doc["outdoor"]["light"] = exteriorData.light;
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServer::handleAPINodes(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(1024);
    JsonArray nodesArray = doc.createNestedArray("nodes");

    if (espnowRcv) {
        // Add interior node
        JsonObject interior = nodesArray.createNestedObject();
        interior["name"] = "Interior Room";
        interior["mac"] = "XX:XX:XX:XX:XX:XX";  // Would get from config
        interior["online"] = espnowRcv->isInteriorOnline();
        interior["rssi"] = -50;  // Would get actual value
        interior["last_packet"] = espnowRcv->getLastInteriorUpdate();

        // Add exterior node
        JsonObject exterior = nodesArray.createNestedObject();
        exterior["name"] = "Exterior Sensors";
        exterior["mac"] = "XX:XX:XX:XX:XX:XX";
        exterior["online"] = espnowRcv->isExteriorOnline();
        exterior["rssi"] = -60;
        exterior["last_packet"] = espnowRcv->getLastExteriorUpdate();
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServer::handleAPIConfig(AsyncWebServerRequest* request) {
    if (!configMgr) {
        request->send(503, "application/json", "{\"error\":\"Config not available\"}");
        return;
    }

    // Get configuration without passwords
    DynamicJsonDocument doc(2048);
    doc["wifi"]["ssid"] = ""; // Read from config
    doc["api"]["latitude"] = 44.4268;
    doc["api"]["longitude"] = 26.1025;
    doc["display"]["brightness"] = 80;
    doc["logging"]["enabled"] = true;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServer::handleAPIConfigSave(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
    // Parse JSON config and save
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, (const char*)data);

    if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    if (configMgr) {
        // configMgr->saveConfig(doc);  // Would call actual save method
    }

    request->send(200, "application/json", "{\"success\":true}");
}

bool WebServer::isAuthenticated(AsyncWebServerRequest* request) {
    // CRITICAL: Check authentication token in header
    if (!request->hasHeader("Authorization")) {
        Serial.println(F("[SECURITY] Unauthorized API access - no auth header"));
        return false;
    }

    String authHeader = request->getHeader("Authorization")->value();

    // Check Bearer token format: "Bearer <token>"
    if (!authHeader.startsWith("Bearer ")) {
        Serial.println(F("[SECURITY] Invalid auth header format"));
        return false;
    }

    // Extract token
    String providedToken = authHeader.substring(7);  // Remove "Bearer "

    // Compare with stored token
    if (strcmp(providedToken.c_str(), ADMIN_TOKEN) != 0) {
        Serial.println(F("[SECURITY] Invalid authentication token"));
        return false;
    }

    Serial.println(F("[SECURITY] API access authenticated"));
    return true;
}

void WebServer::sendJSON(AsyncWebServerRequest* request, JsonDocument& doc) {
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}
