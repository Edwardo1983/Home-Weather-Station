/**
 * @file espnow_receiver.cpp
 * @brief ESP-NOW receiver implementation
 */

#include "espnow_receiver.h"

// Static instance for callback
static ESPNowReceiver* espnowInstance = nullptr;

ESPNowReceiver::ESPNowReceiver() {
  espnowInstance = this;
}

bool ESPNowReceiver::begin() {
  // Initialize WiFi in STA mode
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("[ERROR] ESP-NOW init failed"));
    return false;
  }

  // Register callbacks
  esp_now_register_recv_cb(onDataReceive);
  esp_now_register_send_cb(onDataSent);

  Serial.println(F("[OK] ESP-NOW initialized"));
  return true;
}

void ESPNowReceiver::addPeer(const uint8_t* macAddr) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, macAddr, 6);
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = ESPNOW_ENCRYPT;

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.print(F("[OK] Peer added: "));
    for (int i = 0; i < 6; i++) {
      Serial.print(macAddr[i], HEX);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  } else {
    Serial.println(F("[ERROR] Failed to add peer"));
  }
}

void ESPNowReceiver::requestUpdate() {
  // Send request to all peers
  uint8_t request[] = {0xAA, 0x55};  // Simple handshake pattern

  uint8_t interiorMac[] = ESPNOW_MAC_INTERIOR;
  uint8_t exteriorMac[] = ESPNOW_MAC_EXTERIOR;

  esp_now_send(interiorMac, request, sizeof(request));
  esp_now_send(exteriorMac, request, sizeof(request));
}

void ESPNowReceiver::onDataReceive(const uint8_t* mac, const uint8_t* data, int len) {
  if (espnowInstance == nullptr) return;
  if (len < sizeof(ESPNowData)) return;

  ESPNowData receivedData;
  memcpy(&receivedData, data, sizeof(ESPNowData));

  // Determine which node sent data
  if (strcmp(receivedData.nodeType, "interior") == 0) {
    espnowInstance->lastInteriorData = receivedData;
    espnowInstance->lastInteriorUpdate = millis();

    if (DEBUG_ESPNOW) {
      Serial.print(F("[ESPNOW] Interior: T="));
      Serial.print(receivedData.temperature);
      Serial.print(F("°C H="));
      Serial.print(receivedData.humidity);
      Serial.println(F("%"));
    }
  } else if (strcmp(receivedData.nodeType, "exterior") == 0) {
    espnowInstance->lastExteriorData = receivedData;
    espnowInstance->lastExteriorUpdate = millis();

    if (DEBUG_ESPNOW) {
      Serial.print(F("[ESPNOW] Exterior: T="));
      Serial.print(receivedData.temperature);
      Serial.print(F("°C H="));
      Serial.print(receivedData.humidity);
      Serial.print(F("% P="));
      Serial.print(receivedData.pressure);
      Serial.println(F(" hPa"));
    }
  }
}

void ESPNowReceiver::onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  if (DEBUG_ESPNOW) {
    Serial.print(F("[ESPNOW] Send status: "));
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? F("Success") : F("Failed"));
  }
}
