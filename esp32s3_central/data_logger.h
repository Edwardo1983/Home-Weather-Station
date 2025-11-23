/**
 * @file data_logger.h
 * @brief SD card data logging for ML training
 */

#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>
#include <SD.h>
#include "config.h"

/**
 * CSV record structure for logging
 */
struct CSVRecord {
  String timestamp;        // ISO 8601 format
  float temp_indoor = 0;
  float humidity_indoor = 0;
  float temp_outdoor = 0;
  float humidity_outdoor = 0;
  float pressure = 0;
  float light = 0;
  uint8_t iaq = 0;
};

/**
 * Manages SD card data logging
 */
class DataLogger {
public:
  DataLogger();

  /**
   * Initialize SD card logging
   */
  bool begin();

  /**
   * Write a CSV record to SD card
   */
  bool writeRecord(const CSVRecord& record);

  /**
   * Flush data to SD card
   */
  void flush();

  /**
   * Get total records logged
   */
  unsigned long getRecordCount() const { return recordCount; }

  /**
   * Get current file size
   */
  unsigned long getFileSize() const { return fileSize; }

  /**
   * Check if logging is active
   */
  bool isReady() const { return ready; }

private:
  File logFile;
  String currentFileName;
  bool ready;
  unsigned long recordCount;
  unsigned long fileSize;
  unsigned long lastFlush;

  /**
   * Create or open log file for today
   */
  bool openLogFile();

  /**
   * Check if current file needs rotation (> 1MB or new day)
   */
  bool needsRotation();

  /**
   * Write CSV header
   */
  void writeHeader();

  /**
   * Generate filename based on date
   */
  String generateFileName(unsigned long timestamp);
};

#endif // DATA_LOGGER_H
