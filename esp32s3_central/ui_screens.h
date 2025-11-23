/**
 * @file ui_screens.h
 * @brief UI screen layouts and rendering
 */

#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include <Arduino.h>
#include "config.h"
#include "sensor_manager.h"

// Forward declare structures
struct RemoteSensorData;
struct WeatherData;

/**
 * Manages UI screen layouts for 3 displays
 */
class UIScreens {
public:
  UIScreens();

  /**
   * Set display manager reference
   */
  void setDisplayManager(class DisplayManager* dm);

  /**
   * Draw main screen (Display 1 - Time/Indoor, Display 2 - Weather, Display 3 - Extended)
   */
  void drawMainScreen(const SensorData& local, const RemoteSensorData& remote, const WeatherData& weather);

  /**
   * Draw weather forecast screen
   */
  void drawForecastScreen(const WeatherData& weather);

  /**
   * Draw heart rate screen
   */
  void drawHeartRateScreen(const HeartRateData& hrData);

  /**
   * Draw settings screen
   */
  void drawSettingsScreen();

  /**
   * Draw status indicator
   */
  void drawStatusBar();

private:
  class DisplayManager* displayMgr;

  /**
   * Draw Display 1 (Top) - Time and Indoor Sensors
   */
  void drawDisplay1(const SensorData& local, const RemoteSensorData& remote);

  /**
   * Draw Display 2 (Middle) - Outdoor Weather
   */
  void drawDisplay2(const WeatherData& weather);

  /**
   * Draw Display 3 (Bottom) - Extended Info and Controls
   */
  void drawDisplay3();

  /**
   * Draw temperature trend arrow
   */
  void drawTrendArrow(int x, int y, int trend);

  /**
   * Draw weather icon
   */
  void drawWeatherIcon(int x, int y, uint8_t iconType);

  /**
   * Draw graph of temperature (24h history)
   */
  void drawTemperatureGraph(int x, int y, int width, int height);

  /**
   * Format time string
   */
  String formatTime(unsigned long timestamp);

  /**
   * Format date string
   */
  String formatDate(unsigned long timestamp);
};

#endif // UI_SCREENS_H
