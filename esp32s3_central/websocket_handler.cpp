/**
 * @file websocket_handler.cpp
 * @brief WebSocket handler implementation
 */

#include "websocket_handler.h"

WebSocketHandler wsHandler;

WebSocketHandler::WebSocketHandler() {}

void WebSocketHandler::broadcastSensorData(const DynamicJsonDocument& data) {
    String message = formatMessage("sensor_update", const_cast<DynamicJsonDocument&>(data));
    queueMessage(message);
}

void WebSocketHandler::broadcastSystemStatus(uint32_t uptime, uint32_t heap, int16_t signal) {
    DynamicJsonDocument doc(256);
    doc["uptime"] = uptime;
    doc["heap"] = heap;
    doc["wifi_signal"] = signal;
    doc["temperature"] = temperatureRead();

    String message = formatMessage("system_status", doc);
    queueMessage(message);
}

void WebSocketHandler::broadcastLog(const char* level, const char* module, const char* message) {
    DynamicJsonDocument doc(512);
    doc["level"] = level;
    doc["module"] = module;
    doc["message"] = message;
    doc["timestamp"] = millis();

    queueMessage("{\"type\":\"log_entry\",\"data\":" + serializeJson(doc) + "}");
}

void WebSocketHandler::broadcastForecast(const DynamicJsonDocument& forecast) {
    String message = formatMessage("forecast_update", const_cast<DynamicJsonDocument&>(forecast));
    queueMessage(message);
}

void WebSocketHandler::broadcastMLPrediction(float rainProb, int tempTrend, const char* condition) {
    DynamicJsonDocument doc(256);
    doc["rain_probability"] = rainProb;
    doc["temperature_trend"] = tempTrend;
    doc["general_condition"] = condition;

    String message = formatMessage("ml_prediction", doc);
    queueMessage(message);
}

void WebSocketHandler::queueMessage(const String& payload) {
    if (messageQueue.size() >= MAX_QUEUE_SIZE) {
        messageQueue.pop_front();
    }

    Message msg;
    msg.payload = payload;
    msg.timestamp = millis();
    messageQueue.push_back(msg);
}

String WebSocketHandler::formatMessage(const char* type, const DynamicJsonDocument& data) {
    String json = "{\"type\":\"" + String(type) + "\",\"data\":";
    String dataStr;
    serializeJson(data, dataStr);
    json += dataStr + "}";
    return json;
}

void WebSocketHandler::processQueue() {
    // Would send queued messages to WebSocket clients
    // Clearing old entries to prevent memory buildup
    unsigned long now = millis();
    while (!messageQueue.empty()) {
        if ((now - messageQueue.front().timestamp) > 60000) { // 1 minute old
            messageQueue.pop_front();
        } else {
            break;
        }
    }
}
