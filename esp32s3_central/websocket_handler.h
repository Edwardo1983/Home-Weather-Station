/**
 * @file websocket_handler.h
 * @brief WebSocket message handler for real-time data streaming
 */

#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <deque>

/**
 * Handles WebSocket communication for real-time data updates
 */
class WebSocketHandler {
public:
    WebSocketHandler();

    /**
     * Broadcast sensor data to all connected clients
     */
    void broadcastSensorData(const DynamicJsonDocument& data);

    /**
     * Broadcast system status
     */
    void broadcastSystemStatus(uint32_t uptime, uint32_t heap, int16_t signal);

    /**
     * Broadcast log entry
     */
    void broadcastLog(const char* level, const char* module, const char* message);

    /**
     * Broadcast forecast update
     */
    void broadcastForecast(const DynamicJsonDocument& forecast);

    /**
     * Broadcast ML prediction
     */
    void broadcastMLPrediction(float rainProb, int tempTrend, const char* condition);

    /**
     * Get message queue size
     */
    size_t getQueueSize() const { return messageQueue.size(); }

    /**
     * Process queued messages (call periodically)
     */
    void processQueue();

private:
    // Message queue for buffering
    struct Message {
        String payload;
        unsigned long timestamp;
    };

    std::deque<Message> messageQueue;
    static const size_t MAX_QUEUE_SIZE = 100;

    /**
     * Add message to queue
     */
    void queueMessage(const String& payload);

    /**
     * Format JSON message
     */
    String formatMessage(const char* type, const DynamicJsonDocument& data);
};

extern WebSocketHandler wsHandler;

#endif // WEBSOCKET_HANDLER_H
