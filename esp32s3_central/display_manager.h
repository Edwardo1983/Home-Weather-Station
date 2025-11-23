/**
 * @file display_manager.h
 * @brief Multi-display TFT management for 3x ILI9341 displays
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include "config.h"

// Forward declare TFT_eSPI class
class TFT_eSPI;

/**
 * Manages 3 ILI9341 displays connected to same SPI bus with individual CS pins
 */
class DisplayManager {
public:
  DisplayManager();
  ~DisplayManager();

  /**
   * Initialize all 3 displays
   * @return true if successful
   */
  bool begin();

  /**
   * Select a display (0, 1, or 2)
   */
  void selectDisplay(uint8_t displayNum);

  /**
   * Clear a display with color
   */
  void clear(uint16_t color = 0x0000);

  /**
   * Clear all displays
   */
  void clearAll(uint16_t color = 0x0000);

  /**
   * Set text color
   */
  void setTextColor(uint16_t fg, uint16_t bg = 0x0000);

  /**
   * Set text size
   */
  void setTextSize(uint8_t size);

  /**
   * Set cursor position
   */
  void setCursor(int16_t x, int16_t y);

  /**
   * Print text
   */
  void print(const char* str);
  void print(const String& str);
  void print(float val, uint8_t decimals = 2);
  void print(int val);

  /**
   * Draw line
   */
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

  /**
   * Draw filled rectangle
   */
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

  /**
   * Draw rectangle outline
   */
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

  /**
   * Draw circle
   */
  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);

  /**
   * Fill circle
   */
  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);

  /**
   * Draw pixel
   */
  void drawPixel(int16_t x, int16_t y, uint16_t color);

  /**
   * Set backlight brightness (0-255)
   */
  void setBacklight(uint8_t brightness);

  /**
   * Show error message on first display
   */
  void showError(const char* message);

  /**
   * Update display refresh rate status
   */
  void setRefreshRate(uint16_t fps);

  /**
   * Get current selected display
   */
  uint8_t getCurrentDisplay() { return currentDisplay; }

  /**
   * Check if displays are initialized
   */
  bool isReady() { return initialized; }

private:
  TFT_eSPI* tft[3];  // 3 display objects
  uint8_t currentDisplay;
  bool initialized;
  uint8_t backlightBrightness;

  /**
   * Initialize backlight PWM
   */
  void initBacklight();
};

#endif // DISPLAY_MANAGER_H
