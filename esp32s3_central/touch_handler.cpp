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

  // Static array for efficiency - avoid re-declaring every call
  static const uint8_t irqPin[] = {TOUCH_1_IRQ, TOUCH_2_IRQ, TOUCH_3_IRQ};

  // Check each display for touch
  for (uint8_t i = 0; i < DISPLAY_COUNT; i++) {
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

  // CRITICAL: Validate bounds immediately
  if (display >= DISPLAY_COUNT) {
    Serial.print(F("[ERROR] Invalid display number: "));
    Serial.println(display);
    return event;  // Return empty event
  }

  // Select the appropriate CS pin
  static const uint8_t csPin[] = {TOUCH_1_CS, TOUCH_2_CS, TOUCH_3_CS};
  digitalWrite(csPin[display], LOW);
  delayMicroseconds(10);

  // Read raw coordinates from XPT2046 via SPI
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

  // Read X coordinate
  SPI.write(0x90);  // X position command
  int16_t rawX = SPI.transfer16(0x00) >> 3;

  delayMicroseconds(10);

  // Read Y coordinate
  SPI.write(0xD0);  // Y position command
  int16_t rawY = SPI.transfer16(0x00) >> 3;

  SPI.endTransaction();
  digitalWrite(csPin[display], HIGH);

  // Convert to display coordinates
  calibrateCoordinates(display, rawX, rawY);

  event.x = rawX;
  event.y = rawY;

  return event;
}

void TouchHandler::calibrateCoordinates(uint8_t display, int16_t& x, int16_t& y) {
  if (display >= DISPLAY_COUNT) {
    Serial.print(F("[ERROR] Invalid display in calibrate: "));
    Serial.println(display);
    return;
  }

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
  if (display >= DISPLAY_COUNT) return;

  Serial.print(F("[CALIBRATE] Touch display "));
  Serial.print(display);
  Serial.println(F(" - tap corners in sequence"));

  // TODO: Implement calibration routine
  // Would display prompts and record touch points at corners
}
