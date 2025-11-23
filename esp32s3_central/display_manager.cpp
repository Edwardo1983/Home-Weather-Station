/**
 * @file display_manager.cpp
 * @brief Implementation of multi-display TFT manager
 */

#include "display_manager.h"
#include <TFT_eSPI.h>

DisplayManager::DisplayManager()
  : currentDisplay(0), initialized(false), backlightBrightness(200) {
  for (int i = 0; i < 3; i++) {
    tft[i] = nullptr;
  }
}

DisplayManager::~DisplayManager() {
  for (int i = 0; i < 3; i++) {
    if (tft[i] != nullptr) {
      delete tft[i];
    }
  }
}

bool DisplayManager::begin() {
  // Initialize backlight first
  initBacklight();

  // Note: TFT_eSPI is complex and requires User_Setup.h configuration
  // For now, we provide the structure - actual initialization requires
  // proper TFT_eSPI library setup with pin definitions

  Serial.println("[DISPLAY] DisplayManager initialized");
  initialized = true;
  return true;
}

void DisplayManager::selectDisplay(uint8_t displayNum) {
  if (displayNum > 2) return;
  currentDisplay = displayNum;
}

void DisplayManager::clear(uint16_t color) {
  if (!initialized) return;
  // Implementation depends on TFT_eSPI
  // Typically: tft[currentDisplay]->fillScreen(color);
}

void DisplayManager::clearAll(uint16_t color) {
  for (uint8_t i = 0; i < 3; i++) {
    selectDisplay(i);
    clear(color);
  }
}

void DisplayManager::setTextColor(uint16_t fg, uint16_t bg) {
  if (!initialized) return;
  // tft[currentDisplay]->setTextColor(fg, bg);
}

void DisplayManager::setTextSize(uint8_t size) {
  if (!initialized) return;
  // tft[currentDisplay]->setTextSize(size);
}

void DisplayManager::setCursor(int16_t x, int16_t y) {
  if (!initialized) return;
  // tft[currentDisplay]->setCursor(x, y);
}

void DisplayManager::print(const char* str) {
  if (!initialized) return;
  // tft[currentDisplay]->print(str);
}

void DisplayManager::print(const String& str) {
  if (!initialized) return;
  // tft[currentDisplay]->print(str);
}

void DisplayManager::print(float val, uint8_t decimals) {
  if (!initialized) return;
  // tft[currentDisplay]->print(val, decimals);
}

void DisplayManager::print(int val) {
  if (!initialized) return;
  // tft[currentDisplay]->print(val);
}

void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  if (!initialized) return;
  // tft[currentDisplay]->drawLine(x0, y0, x1, y1, color);
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (!initialized) return;
  // tft[currentDisplay]->fillRect(x, y, w, h, color);
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (!initialized) return;
  // tft[currentDisplay]->drawRect(x, y, w, h, color);
}

void DisplayManager::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
  if (!initialized) return;
  // tft[currentDisplay]->drawCircle(x, y, r, color);
}

void DisplayManager::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
  if (!initialized) return;
  // tft[currentDisplay]->fillCircle(x, y, r, color);
}

void DisplayManager::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (!initialized) return;
  // tft[currentDisplay]->drawPixel(x, y, color);
}

void DisplayManager::setBacklight(uint8_t brightness) {
  backlightBrightness = brightness;
  ledcWrite(0, brightness);  // PWM channel 0
}

void DisplayManager::showError(const char* message) {
  if (!initialized) return;

  for (uint8_t i = 0; i < 3; i++) {
    selectDisplay(i);
    clear(0xF800);  // Red
    setTextColor(0xFFFF, 0xF800);
    setTextSize(1);
    setCursor(10, 50);
    print("ERROR:");
    setCursor(10, 80);
    print(message);
  }
}

void DisplayManager::setRefreshRate(uint16_t fps) {
  // Informational - actual refresh rate depends on
  // how frequently update functions are called
  Serial.print("[DISPLAY] Target FPS: ");
  Serial.println(fps);
}

void DisplayManager::initBacklight() {
  // Initialize PWM for backlight
  // Channel 0, frequency 5000 Hz, resolution 8 bits
  ledcSetup(0, 5000, 8);
  ledcAttachPin(DISPLAY_LED, 0);
  ledcWrite(0, backlightBrightness);
  Serial.println("[DISPLAY] Backlight PWM initialized");
}
