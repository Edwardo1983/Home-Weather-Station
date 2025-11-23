/**
 * @file ml_predictor.h
 * @brief ML-based weather prediction using collected data
 */

#ifndef ML_PREDICTOR_H
#define ML_PREDICTOR_H

#include <Arduino.h>
#include "config.h"
#include "sensor_manager.h"

/**
 * Weather prediction data
 */
struct WeatherPrediction {
  float rainProbability = 0;        // 0-100%
  int temperatureTrend = 0;         // -1=falling, 0=stable, 1=rising
  String generalCondition = "";     // "sunny", "cloudy", "rainy", "stormy"
  unsigned long timestamp = 0;
};

/**
 * Simple rule-based weather prediction
 * Can be enhanced with TinyML models trained on logged data
 */
class MLPredictor {
public:
  MLPredictor();

  /**
   * Initialize predictor
   */
  void begin();

  /**
   * Update prediction based on current sensor data
   */
  void update(const SensorData& current);

  /**
   * Get latest prediction
   */
  WeatherPrediction getPrediction() const { return lastPrediction; }

  /**
   * Train model from CSV data (would be called offline)
   */
  void trainFromCSV(const char* csvFileName);

private:
  WeatherPrediction lastPrediction;

  // Historical data for trend analysis
  struct HistoricalPoint {
    float pressure;
    float temperature;
    float humidity;
    unsigned long timestamp;
  } history[144];  // 12 hours of 5-minute samples

  uint8_t historyIndex;
  uint8_t historyFilled;

  /**
   * Calculate pressure trend (falling = rain)
   */
  int calculatePressureTrend();

  /**
   * Calculate temperature trend
   */
  int calculateTemperatureTrend();

  /**
   * Estimate rain probability from features
   */
  float estimateRainProbability();

  /**
   * Rule-based weather classification
   */
  String classifyWeather();

  /**
   * Add data point to history
   */
  void addHistoricalPoint(const SensorData& data);
};

#endif // ML_PREDICTOR_H
