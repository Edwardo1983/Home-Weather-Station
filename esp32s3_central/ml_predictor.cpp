/**
 * @file ml_predictor.cpp
 * @brief ML predictor implementation
 */

#include "ml_predictor.h"

MLPredictor::MLPredictor() : historyIndex(0), historyFilled(0) {
  memset(history, 0, sizeof(history));
}

void MLPredictor::begin() {
  Serial.println(F("[ML] Predictor initialized"));
}

void MLPredictor::update(const SensorData& current) {
  // Add current data to history
  addHistoricalPoint(current);

  // Calculate predictions
  lastPrediction.rainProbability = estimateRainProbability();
  lastPrediction.temperatureTrend = calculateTemperatureTrend();
  lastPrediction.generalCondition = classifyWeather();
  lastPrediction.timestamp = millis();

  if (DEBUG_SENSORS) {
    Serial.print(F("[ML] Rain prob: "));
    Serial.print(lastPrediction.rainProbability);
    Serial.print(F("% Condition: "));
    Serial.println(lastPrediction.generalCondition);
  }
}

void MLPredictor::addHistoricalPoint(const SensorData& data) {
  history[historyIndex].pressure = data.pressure;
  history[historyIndex].temperature = data.temperature;
  history[historyIndex].humidity = data.humidity;
  history[historyIndex].timestamp = millis();

  historyIndex = (historyIndex + 1) % 144;
  if (historyFilled < 144) historyFilled++;
}

int MLPredictor::calculatePressureTrend() {
  if (historyFilled < 12) return 0;  // Not enough data

  // Compare current pressure with 1 hour ago
  float currentPressure = history[historyIndex].pressure;
  float pressureHourAgo = history[(historyIndex - 12 + 144) % 144].pressure;

  float delta = currentPressure - pressureHourAgo;

  if (delta < -5.0) return -1;  // Falling rapidly = rain likely
  if (delta < -1.0) return -1;  // Falling
  if (delta > 1.0) return 1;    // Rising = better weather
  return 0;  // Stable
}

int MLPredictor::calculateTemperatureTrend() {
  if (historyFilled < 6) return 0;

  float currentTemp = history[historyIndex].temperature;
  float tempBefore = history[(historyIndex - 6 + 144) % 144].temperature;

  float delta = currentTemp - tempBefore;

  if (delta > 0.5) return 1;   // Rising
  if (delta < -0.5) return -1; // Falling
  return 0;  // Stable
}

float MLPredictor::estimateRainProbability() {
  if (historyFilled < 12) return 0;

  float rainProb = 0;

  // Factor 1: Pressure trend (decreasing = rain likely)
  int pressureTrend = calculatePressureTrend();
  if (pressureTrend < 0) {
    rainProb += 40;  // 40% if pressure falling
  }

  // Factor 2: Humidity (high humidity = rain possible)
  float avgHumidity = 0;
  for (int i = 0; i < historyFilled; i++) {
    avgHumidity += history[i].humidity;
  }
  avgHumidity /= historyFilled;

  if (avgHumidity > 75) {
    rainProb += (avgHumidity - 75) * 1.6;  // Up to 40% for very high humidity
  }

  // Factor 3: Temperature trend rapid change = unstable weather
  int tempTrend = calculateTemperatureTrend();
  if (tempTrend != 0) {
    rainProb += 20;
  }

  // Clamp to 0-100
  if (rainProb > 100) rainProb = 100;
  if (rainProb < 0) rainProb = 0;

  return rainProb;
}

String MLPredictor::classifyWeather() {
  float rainProb = lastPrediction.rainProbability;

  if (rainProb > 60) {
    return "Rainy";
  } else if (rainProb > 30) {
    return "Cloudy";
  } else if (rainProb > 10) {
    return "Partly Cloudy";
  } else {
    return "Sunny";
  }
}

void MLPredictor::trainFromCSV(const char* csvFileName) {
  // TODO: Implement CSV loading and model training
  // This would be called offline with historical data
  Serial.print(F("[ML] Training from: "));
  Serial.println(csvFileName);
}
