/**
 * @file touch_handler.h
 * @brief Touch screen input handling (XPT2046)
 */

#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

#include <Arduino.h>
#include "config.h"

/**
 * Touch event structure
 */
struct TouchEvent {
  enum Type { TOUCH_NONE, TOUCH_PRESS, TOUCH_RELEASE, TOUCH_DRAG };

  Type type = TOUCH_NONE;
  uint8_t display = 0;  // Which display (0, 1, or 2)
  int16_t x = -1;
  int16_t y = -1;
  unsigned long timestamp = 0;
};

/**
 * Manages XPT2046 touch input from 3 displays
 */
class TouchHandler {
public:
  TouchHandler();

  /**
   * Initialize touch inputs
   * @return true if successful
   */
  bool begin();

  /**
   * Get latest touch event
   */
  TouchEvent getEvent();

  /**
   * Calibrate touch screen
   */
  void calibrate(uint8_t display);

  /**
   * Enable/disable touch
   */
  void setEnabled(bool enabled) { enabled_ = enabled; }

private:
  bool enabled_;
  TouchEvent lastEvent;

  /**
   * Read raw touch coordinates
   */
  TouchEvent readTouch(uint8_t display);

  /**
   * Convert raw to calibrated coordinates
   */
  void calibrateCoordinates(uint8_t display, int16_t& x, int16_t& y);

  // Calibration data for each display
  struct CalibrationData {
    float xMin = 100, xMax = 3950;
    float yMin = 100, yMax = 3950;
  } calib[3];
};

#endif // TOUCH_HANDLER_H
