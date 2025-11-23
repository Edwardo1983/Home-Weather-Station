/**
 * @file ui_screens.cpp
 * @brief UI screen rendering implementation
 */

#include "ui_screens.h"
#include "display_manager.h"

UIScreens::UIScreens() : displayMgr(nullptr) {}

void UIScreens::setDisplayManager(DisplayManager* dm) {
  displayMgr = dm;
}

void UIScreens::drawMainScreen(const SensorData& local, const RemoteSensorData& remote, const WeatherData& weather) {
  if (displayMgr == nullptr) return;

  // Clear all displays
  displayMgr->clearAll(0x0000);

  // Draw on each display
  drawDisplay1(local, remote);
  drawDisplay2(weather);
  drawDisplay3();
}

void UIScreens::drawDisplay1(const SensorData& local, const RemoteSensorData& remote) {
  if (displayMgr == nullptr) return;

  displayMgr->selectDisplay(0);
  displayMgr->setTextColor(0xFFFF, 0x0000);  // White on black
  displayMgr->setTextSize(2);

  // Time and date (large)
  displayMgr->setCursor(10, 10);
  displayMgr->print(formatTime(millis()));

  displayMgr->setTextSize(1);
  displayMgr->setCursor(10, 40);
  displayMgr->print(formatDate(millis()));

  // Indoor sensors (this room)
  displayMgr->setCursor(10, 70);
  displayMgr->print(F("Indoor Main:"));
  displayMgr->setCursor(10, 90);
  displayMgr->print(F("T: "));
  displayMgr->print(local.temperature, 1);
  displayMgr->print(F("C"));

  displayMgr->setCursor(10, 110);
  displayMgr->print(F("H: "));
  displayMgr->print(local.humidity, 0);
  displayMgr->print(F("%"));

  displayMgr->setCursor(10, 130);
  displayMgr->print(F("IAQ: "));
  displayMgr->print(local.iaq);

  // Secondary room
  displayMgr->setCursor(10, 160);
  displayMgr->print(F("Indoor Sec:"));
  displayMgr->setCursor(10, 180);
  displayMgr->print(F("T: "));
  displayMgr->print(remote.temp_interior, 1);
  displayMgr->print(F("C"));

  displayMgr->setCursor(10, 200);
  displayMgr->print(F("H: "));
  displayMgr->print(remote.humidity_interior, 0);
  displayMgr->print(F("%"));

  // Status indicators
  displayMgr->setTextSize(1);
  displayMgr->setCursor(240, 220);
  displayMgr->setTextColor(0x00FF00, 0x0000);  // Green for online
  displayMgr->print(F("OK"));
}

void UIScreens::drawDisplay2(const WeatherData& weather) {
  if (displayMgr == nullptr) return;

  displayMgr->selectDisplay(1);
  displayMgr->setTextColor(0xFFFF, 0x0000);
  displayMgr->setTextSize(2);

  // Current weather title
  displayMgr->setCursor(10, 10);
  displayMgr->print(F("Outdoor"));

  displayMgr->setTextSize(3);
  displayMgr->setCursor(50, 50);
  displayMgr->print(weather.currentTemp, 0);
  displayMgr->print(F("C"));

  displayMgr->setTextSize(1);
  displayMgr->setCursor(10, 120);
  displayMgr->print(weather.description);

  displayMgr->setCursor(10, 140);
  displayMgr->print(F("Feels: "));
  displayMgr->print(weather.feelsLike, 1);
  displayMgr->print(F("C"));

  displayMgr->setCursor(10, 160);
  displayMgr->print(F("Humidity: "));
  displayMgr->print(weather.humidity);
  displayMgr->print(F("%"));

  displayMgr->setCursor(10, 180);
  displayMgr->print(F("Pressure: "));
  displayMgr->print(weather.pressure, 0);
  displayMgr->print(F(" hPa"));

  displayMgr->setCursor(10, 200);
  displayMgr->print(F("Wind: "));
  displayMgr->print(weather.windSpeed, 1);
  displayMgr->print(F(" m/s"));
}

void UIScreens::drawDisplay3() {
  if (displayMgr == nullptr) return;

  displayMgr->selectDisplay(2);
  displayMgr->setTextColor(0xFFFF, 0x0000);
  displayMgr->setTextSize(1);

  // Extended info or controls
  displayMgr->setCursor(10, 10);
  displayMgr->print(F("Extended Data"));

  displayMgr->setCursor(10, 30);
  displayMgr->print(F("24h Temperature Graph"));

  // Draw simple separator line
  displayMgr->drawLine(10, 25, 310, 25, 0x4208);

  // Buttons area at bottom
  displayMgr->drawRect(10, 200, 70, 30, 0x07E0);   // Left button (green)
  displayMgr->drawRect(125, 200, 70, 30, 0xF800);  // Center button (red)
  displayMgr->drawRect(240, 200, 70, 30, 0x07E0);  // Right button (green)

  displayMgr->setCursor(20, 210);
  displayMgr->print(F("SETTINGS"));

  displayMgr->setCursor(135, 210);
  displayMgr->print(F("REFRESH"));

  displayMgr->setCursor(255, 210);
  displayMgr->print(F("HOME"));
}

void UIScreens::drawForecastScreen(const WeatherData& weather) {
  if (displayMgr == nullptr) return;

  displayMgr->selectDisplay(1);
  displayMgr->clearAll(0x0000);
  displayMgr->setTextColor(0xFFFF, 0x0000);
  displayMgr->setTextSize(1);

  displayMgr->setCursor(10, 10);
  displayMgr->print(F("5-Day Forecast"));

  displayMgr->drawLine(10, 25, 310, 25, 0x4208);

  // Draw forecast cards (simplified)
  for (int i = 0; i < 5; i++) {
    int xPos = 10 + (i * 60);
    int yPos = 40;

    displayMgr->setCursor(xPos, yPos);
    displayMgr->print(F("Day"));
    displayMgr->print(i + 1);

    displayMgr->setCursor(xPos, yPos + 20);
    displayMgr->print(weather.forecastTemp[i], 0);
    displayMgr->print(F("C"));

    displayMgr->setCursor(xPos, yPos + 40);
    displayMgr->print(weather.forecastRain[i] * 100, 0);
    displayMgr->print(F("%"));
  }
}

void UIScreens::drawHeartRateScreen(const HeartRateData& hrData) {
  if (displayMgr == nullptr) return;

  displayMgr->selectDisplay(2);
  displayMgr->clearAll(0x0000);
  displayMgr->setTextColor(0xF800, 0x0000);  // Red text
  displayMgr->setTextSize(3);

  displayMgr->setCursor(50, 50);
  displayMgr->print(hrData.heartRate);
  displayMgr->print(F(" BPM"));

  displayMgr->setTextSize(2);
  displayMgr->setCursor(60, 120);
  displayMgr->print(F("SpO2: "));
  displayMgr->print(hrData.spo2);
  displayMgr->print(F("%"));

  if (!hrData.isValid) {
    displayMgr->setTextColor(0xFFFF, 0x0000);
    displayMgr->setCursor(50, 170);
    displayMgr->print(F("Place finger on sensor"));
  }
}

void UIScreens::drawTrendArrow(int x, int y, int trend) {
  if (displayMgr == nullptr) return;

  if (trend > 0) {
    // Rising arrow
    displayMgr->drawLine(x, y, x - 5, y + 10, 0x07E0);  // Green
    displayMgr->drawLine(x, y, x + 5, y + 10, 0x07E0);
  } else if (trend < 0) {
    // Falling arrow
    displayMgr->drawLine(x, y, x - 5, y - 10, 0xF800);  // Red
    displayMgr->drawLine(x, y, x + 5, y - 10, 0xF800);
  } else {
    // Horizontal line (stable)
    displayMgr->drawLine(x - 5, y, x + 5, y, 0xFFFF);  // White
  }
}

void UIScreens::drawWeatherIcon(int x, int y, uint8_t iconType) {
  // Simple icon drawing (circles and lines for now)
  // 0=sunny, 1=cloudy, 2=rainy, 3=thunderstorm, 4=snow, 5=foggy

  switch (iconType) {
    case 0:  // Sunny
      displayMgr->fillCircle(x, y, 8, 0xFFE0);  // Yellow
      break;
    case 1:  // Cloudy
      displayMgr->fillCircle(x, y, 8, 0xBDF7);  // Light gray
      break;
    case 2:  // Rainy
      displayMgr->drawCircle(x, y, 8, 0x001F);  // Blue outline
      break;
    case 3:  // Thunderstorm
      displayMgr->drawLine(x, y - 5, x, y + 5, 0xFFE0);  // Yellow line
      break;
  }
}

void UIScreens::drawTemperatureGraph(int x, int y, int width, int height) {
  // TODO: Implement 24-hour temperature graph
  // Would need to store hourly temperature data
  displayMgr->drawRect(x, y, width, height, 0xFFFF);
}

String UIScreens::formatTime(unsigned long timestamp) {
  // Simple time formatting (would need proper time source)
  unsigned long seconds = (timestamp / 1000) % 60;
  unsigned long minutes = (timestamp / 60000) % 60;
  unsigned long hours = (timestamp / 3600000) % 24;

  String timeStr = "";
  if (hours < 10) timeStr += "0";
  timeStr += String(hours);
  timeStr += ":";
  if (minutes < 10) timeStr += "0";
  timeStr += String(minutes);

  return timeStr;
}

String UIScreens::formatDate(unsigned long timestamp) {
  // TODO: Implement proper date from RTC
  return String("Date: TBD");
}
