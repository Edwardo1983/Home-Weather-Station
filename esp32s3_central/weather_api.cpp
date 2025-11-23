/**
 * @file weather_api.cpp
 * @brief Weather API implementation
 */

#include "weather_api.h"

WeatherAPI::WeatherAPI() {}

void WeatherAPI::setAPIKeys(const char* owmKey, const char* tomorrowKey) {
  owmApiKey = String(owmKey);
  tomorrowApiKey = String(tomorrowKey);
}

bool WeatherAPI::fetchCurrentWeather(float lat, float lon) {
  if (owmApiKey.length() == 0) {
    Serial.println(F("[ERROR] OpenWeatherMap API key not set"));
    return false;
  }

  String url = String(OWM_CURRENT_URL) +
               "?lat=" + String(lat, 6) +
               "&lon=" + String(lon, 6) +
               "&units=metric&appid=" + owmApiKey;

  String response = httpGET(url);
  if (response.length() == 0) {
    Serial.println(F("[ERROR] Failed to fetch current weather"));
    return false;
  }

  return parseCurrentWeatherJSON(response);
}

bool WeatherAPI::fetchForecast(float lat, float lon) {
  if (owmApiKey.length() == 0) {
    Serial.println(F("[ERROR] OpenWeatherMap API key not set"));
    return false;
  }

  String url = String(OWM_FORECAST_URL) +
               "?lat=" + String(lat, 6) +
               "&lon=" + String(lon, 6) +
               "&units=metric&appid=" + owmApiKey;

  String response = httpGET(url);
  if (response.length() == 0) {
    Serial.println(F("[ERROR] Failed to fetch forecast"));
    return false;
  }

  return parseForecastJSON(response);
}

String WeatherAPI::httpGET(const String& url) {
  HTTPClient http;
  http.setTimeout(WEATHER_API_TIMEOUT);

  if (!http.begin(url)) {
    Serial.println(F("[ERROR] Failed to connect to HTTP"));
    return "";
  }

  int httpCode = http.GET();
  String response = "";

  if (httpCode == HTTP_CODE_OK) {
    response = http.getString();
  } else {
    Serial.print(F("[ERROR] HTTP status: "));
    Serial.println(httpCode);
  }

  http.end();
  return response;
}

bool WeatherAPI::parseCurrentWeatherJSON(const String& jsonStr) {
  DynamicJsonDocument doc(2048);

  DeserializationError error = deserializeJson(doc, jsonStr);
  if (error) {
    Serial.print(F("[ERROR] JSON parse error: "));
    Serial.println(error.c_str());
    return false;
  }

  currentWeather.temp = doc["main"]["temp"] | 0;
  currentWeather.feelsLike = doc["main"]["feels_like"] | 0;
  currentWeather.humidity = doc["main"]["humidity"] | 0;
  currentWeather.pressure = doc["main"]["pressure"] | 0;
  currentWeather.cloudiness = doc["clouds"]["all"] | 0;
  currentWeather.windSpeed = doc["wind"]["speed"] | 0;
  currentWeather.visibility = doc["visibility"] | 0;

  if (doc["weather"].size() > 0) {
    currentWeather.weatherCode = doc["weather"][0]["id"] | 0;
    currentWeather.description = doc["weather"][0]["main"].as<String>();
  }

  currentWeather.uvIndex = doc["uvi"] | 0;

  return true;
}

bool WeatherAPI::parseForecastJSON(const String& jsonStr) {
  DynamicJsonDocument doc(4096);

  DeserializationError error = deserializeJson(doc, jsonStr);
  if (error) {
    Serial.print(F("[ERROR] Forecast JSON parse error: "));
    Serial.println(error.c_str());
    return false;
  }

  // Parse 5-day forecast (3-hour intervals, take every 8th entry = daily)
  JsonArray list = doc["list"];
  int forecastIndex = 0;

  for (int i = 0; i < list.size() && forecastIndex < 5; i += 8) {
    JsonObject item = list[i];
    forecast[forecastIndex].tempMax = item["main"]["temp_max"] | 0;
    forecast[forecastIndex].tempMin = item["main"]["temp_min"] | 0;
    forecast[forecastIndex].humidity = item["main"]["humidity"] | 0;
    forecast[forecastIndex].rainProbability = item["pop"] | 0;

    if (item["weather"].size() > 0) {
      forecast[forecastIndex].weatherCode = item["weather"][0]["id"] | 0;
    }

    if (item["rain"] && item["rain"]["3h"]) {
      forecast[forecastIndex].rainfall = item["rain"]["3h"] | 0;
    }

    forecastIndex++;
  }

  return true;
}

uint8_t WeatherAPI::mapWeatherCode(int owmCode) {
  // Map OpenWeatherMap codes to icon indices
  // 0=sunny, 1=cloudy, 2=rainy, 3=thunderstorm, 4=snow, 5=foggy
  if (owmCode >= 200 && owmCode < 300) return 3;  // Thunderstorm
  if (owmCode >= 300 && owmCode < 400) return 2;  // Drizzle
  if (owmCode >= 500 && owmCode < 600) return 2;  // Rain
  if (owmCode >= 600 && owmCode < 700) return 4;  // Snow
  if (owmCode >= 700 && owmCode < 800) return 5;  // Fog
  if (owmCode == 800) return 0;                   // Clear/Sunny
  if (owmCode > 800) return 1;                    // Clouds

  return 1;  // Default to cloudy
}

String WeatherAPI::getWeatherDescription(int owmCode) {
  if (owmCode >= 200 && owmCode < 300) return "Thunderstorm";
  if (owmCode >= 300 && owmCode < 400) return "Drizzle";
  if (owmCode >= 500 && owmCode < 600) return "Rainy";
  if (owmCode >= 600 && owmCode < 700) return "Snowy";
  if (owmCode >= 700 && owmCode < 800) return "Foggy";
  if (owmCode == 800) return "Clear";
  if (owmCode == 801) return "Partly Cloudy";
  if (owmCode > 801) return "Cloudy";

  return "Unknown";
}

uint8_t WeatherAPI::getWeatherIcon(int weatherCode) {
  return mapWeatherCode(weatherCode);
}
