/**
 * @file touch_handler.cpp
 * @brief Touch handler implementation
 */

#include "touch_handler.h"

TouchHandler::TouchHandler() : enabled_(false) {}

bool TouchHandler::begin() {
  // Initialize GPIO pins for touch CS and IRQ
  pinMode(TOUCH_1_CS, OUTPUT);
  pinMode(TOUCH_2_CS, OUTPUT);
  pinMode(TOUCH_3_CS, OUTPUT);
  pinMode(TOUCH_1_IRQ, INPUT);
  pinMode(TOUCH_2_IRQ, INPUT);
  pinMode(TOUCH_3_IRQ, INPUT);

  digitalWrite(TOUCH_1_CS, HIGH);
  digitalWrite(TOUCH_2_CS, HIGH);
  digitalWrite(TOUCH_3_CS, HIGH);

  enabled_ = true;
  Serial.println(F("[OK] Touch handler initialized"));
  return true;
}

TouchEvent TouchHandler::getEvent() {
  if (!enabled_) {
    return TouchEvent();
  }

  TouchEvent event;

  // Check each display for touch
  for (uint8_t i = 0; i < 3; i++) {
    // Read IRQ pins to detect touch
    uint8_t irqPin[] = {TOUCH_1_IRQ, TOUCH_2_IRQ, TOUCH_3_IRQ};

    if (digitalRead(irqPin[i]) == LOW) {  // Touch active
      event = readTouch(i);
      event.display = i;
      event.type = TouchEvent::TOUCH_PRESS;
      event.timestamp = millis();
      return event;
    }
  }

  return event;
}

TouchEvent TouchHandler::readTouch(uint8_t display) {
  TouchEvent event;
  event.display = display;

  // Select the appropriate CS pin
  uint8_t csPin[] = {TOUCH_1_CS, TOUCH_2_CS, TOUCH_3_CS};
  digitalWrite(csPin[display], LOW);

  delay(1);

  // Read raw coordinates from XPT2046
  // This is simplified - full implementation would use SPI communication
  int16_t rawX = 0, rawY = 0;

  // TODO: Implement proper XPT2046 SPI communication

  // Convert to display coordinates
  calibrateCoordinates(display, rawX, rawY);

  event.x = rawX;
  event.y = rawY;

  digitalWrite(csPin[display], HIGH);

  return event;
}

void TouchHandler::calibrateCoordinates(uint8_t display, int16_t& x, int16_t& y) {
  if (display >= 3) return;

  CalibrationData& cal = calib[display];

  // Scale raw coordinates (0-4095) to display coordinates (0-319 x 0-239)
  x = (int16_t)(((float)(x - cal.xMin) / (cal.xMax - cal.xMin)) * DISPLAY_WIDTH);
  y = (int16_t)(((float)(y - cal.yMin) / (cal.yMax - cal.yMin)) * DISPLAY_HEIGHT);

  // Clamp to display boundaries
  if (x < 0) x = 0;
  if (x >= DISPLAY_WIDTH) x = DISPLAY_WIDTH - 1;
  if (y < 0) y = 0;
  if (y >= DISPLAY_HEIGHT) y = DISPLAY_HEIGHT - 1;
}

void TouchHandler::calibrate(uint8_t display) {
  if (display >= 3) return;

  Serial.print(F("[CALIBRATE] Touch display "));
  Serial.print(display);
  Serial.println(F(" - tap corners in sequence"));

  // TODO: Implement calibration routine
  // Would display prompts and record touch points at corners
}
