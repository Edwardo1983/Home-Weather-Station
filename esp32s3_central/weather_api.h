/**
 * @file weather_api.h
 * @brief Weather API integration (OpenWeatherMap, Tomorrow.io)
 */

#ifndef WEATHER_API_H
#define WEATHER_API_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/**
 * Current weather data structure
 */
struct CurrentWeather {
  float temp = 0;
  float feelsLike = 0;
  int humidity = 0;
  float pressure = 0;
  int weatherCode = 0;
  String description = "";
  float windSpeed = 0;
  float uvIndex = 0;
  int cloudiness = 0;
  float visibility = 0;
};

/**
 * Forecast data for one day
 */
struct ForecastDay {
  float tempMax = 0;
  float tempMin = 0;
  int weatherCode = 0;
  float rainProbability = 0;
  float rainfall = 0;
  int humidity = 0;
  float windSpeed = 0;
};

/**
 * Manages weather API calls
 */
class WeatherAPI {
public:
  WeatherAPI();

  /**
   * Set API keys
   */
  void setAPIKeys(const char* owmKey, const char* tomorrowKey);

  /**
   * Fetch current weather from OpenWeatherMap
   * @return true if successful
   */
  bool fetchCurrentWeather(float lat, float lon);

  /**
   * Fetch 5-day forecast from OpenWeatherMap
   * @return true if successful
   */
  bool fetchForecast(float lat, float lon);

  /**
   * Get current weather data
   */
  CurrentWeather getCurrentWeather() const { return currentWeather; }

  /**
   * Get forecast data
   */
  ForecastDay* getForecast() { return forecast; }

  /**
   * Get weather icon code for display
   */
  uint8_t getWeatherIcon(int weatherCode);

  /**
   * Get human-readable weather description
   */
  String getWeatherDescription(int weatherCode);

private:
  String owmApiKey;
  String tomorrowApiKey;
  CurrentWeather currentWeather;
  ForecastDay forecast[5];

  /**
   * Parse JSON from OpenWeatherMap current weather
   */
  bool parseCurrentWeatherJSON(const String& jsonStr);

  /**
   * Parse JSON from OpenWeatherMap forecast
   */
  bool parseForecastJSON(const String& jsonStr);

  /**
   * Make HTTP GET request
   */
  String httpGET(const String& url);

  /**
   * Map OpenWeatherMap code to icon
   */
  uint8_t mapWeatherCode(int owmCode);
};

#endif // WEATHER_API_H
