/**
 * @file display_manager.cpp
 * @brief Implementation of multi-display TFT manager
 */

#include "display_manager.h"
#include <TFT_eSPI.h>

// Create 3 TFT instances for each display
TFT_eSPI tft1 = TFT_eSPI();
TFT_eSPI tft2 = TFT_eSPI();
TFT_eSPI tft3 = TFT_eSPI();

DisplayManager::DisplayManager()
  : currentDisplay(0), initialized(false), backlightBrightness(200) {
  tft[0] = &tft1;
  tft[1] = &tft2;
  tft[2] = &tft3;
}

DisplayManager::~DisplayManager() {
  // Note: Static TFT instances - don't delete
}

bool DisplayManager::begin() {
  // Initialize backlight first
  initBacklight();

  // Initialize each display with its CS pin
  for (int i = 0; i < 3; i++) {
    selectDisplay(i);
    tft[i]->init();
    tft[i]->setRotation(DISPLAY_ROTATION);
    tft[i]->fillScreen(TFT_BLACK);
    delay(100);
  }

  // Select first display by default
  selectDisplay(0);

  Serial.println("[DISPLAY] All 3 displays initialized");
  initialized = true;
  return true;
}

void DisplayManager::selectDisplay(uint8_t displayNum) {
  if (displayNum > 2) return;

  // Deselect all displays
  digitalWrite(DISPLAY_1_CS, HIGH);
  digitalWrite(DISPLAY_2_CS, HIGH);
  digitalWrite(DISPLAY_3_CS, HIGH);

  // Select requested display
  currentDisplay = displayNum;
  switch (displayNum) {
    case 0: digitalWrite(DISPLAY_1_CS, LOW); break;
    case 1: digitalWrite(DISPLAY_2_CS, LOW); break;
    case 2: digitalWrite(DISPLAY_3_CS, LOW); break;
  }

  delayMicroseconds(10);  // CS setup time
}

void DisplayManager::clear(uint16_t color) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->fillScreen(color);
}

void DisplayManager::clearAll(uint16_t color) {
  for (uint8_t i = 0; i < 3; i++) {
    selectDisplay(i);
    clear(color);
  }
}

void DisplayManager::setTextColor(uint16_t fg, uint16_t bg) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->setTextColor(fg, bg);
}

void DisplayManager::setTextSize(uint8_t size) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->setTextSize(size);
}

void DisplayManager::setCursor(int16_t x, int16_t y) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->setCursor(x, y);
}

void DisplayManager::print(const char* str) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->print(str);
}

void DisplayManager::print(const String& str) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->print(str);
}

void DisplayManager::print(float val, uint8_t decimals) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->print(val, decimals);
}

void DisplayManager::print(int val) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->print(val);
}

void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->drawLine(x0, y0, x1, y1, color);
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->fillRect(x, y, w, h, color);
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->drawRect(x, y, w, h, color);
}

void DisplayManager::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->drawCircle(x, y, r, color);
}

void DisplayManager::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->fillCircle(x, y, r, color);
}

void DisplayManager::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (!initialized || tft[currentDisplay] == nullptr) return;
  tft[currentDisplay]->drawPixel(x, y, color);
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
