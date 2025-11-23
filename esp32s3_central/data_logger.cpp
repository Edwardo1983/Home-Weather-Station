/**
 * @file data_logger.cpp
 * @brief Data logging implementation
 */

#include "data_logger.h"
#include "utils.h"

DataLogger::DataLogger()
  : ready(false), recordCount(0), fileSize(0), lastFlush(0) {}

bool DataLogger::begin() {
  // SD card is expected to be initialized before calling this
  if (!SD.exists("/")) {
    Serial.println(F("[ERROR] SD card not accessible"));
    return false;
  }

  ready = openLogFile();
  return ready;
}

bool DataLogger::openLogFile() {
  currentFileName = generateFileName(millis());

  // Check if file exists
  bool isNewFile = !SD.exists(currentFileName.c_str());

  // Open file in append mode
  logFile = SD.open(currentFileName.c_str(), FILE_APPEND);

  if (!logFile) {
    Serial.print(F("[ERROR] Failed to open log file: "));
    Serial.println(currentFileName);
    return false;
  }

  // Write header if new file
  if (isNewFile && ENABLE_CSV_HEADER) {
    writeHeader();
  }

  fileSize = logFile.size();
  Serial.print(F("[OK] Log file opened: "));
  Serial.print(currentFileName);
  Serial.print(F(" Size: "));
  Serial.println(fileSize);

  return true;
}

bool DataLogger::writeRecord(const CSVRecord& record) {
  if (!ready || !logFile) return false;

  // Check if rotation needed
  if (needsRotation()) {
    logFile.close();
    if (!openLogFile()) {
      return false;
    }
  }

  // Format CSV line
  String line = record.timestamp + ",";
  line += String(record.temp_indoor, 2) + ",";
  line += String(record.humidity_indoor, 2) + ",";
  line += String(record.temp_outdoor, 2) + ",";
  line += String(record.humidity_outdoor, 2) + ",";
  line += String(record.pressure, 2) + ",";
  line += String(record.light, 2) + ",";
  line += String(record.iaq);
  line += "\n";

  // Write to file
  size_t written = logFile.print(line);

  if (written == line.length()) {
    recordCount++;
    fileSize += written;

    // Flush every 10 records or every 5 minutes
    if (recordCount % 10 == 0 || (millis() - lastFlush) > 300000) {
      flush();
    }

    return true;
  }

  return false;
}

void DataLogger::flush() {
  if (!logFile) return;

  logFile.flush();
  lastFlush = millis();

  Serial.print(F("[LOG] Flushed "));
  Serial.print(recordCount);
  Serial.print(F(" records, file size: "));
  Serial.println(fileSize);
}

bool DataLogger::needsRotation() {
  // Rotate if file exceeds 1MB or if day has changed
  if (fileSize > LOG_FILE_MAX_SIZE) {
    Serial.println(F("[LOG] File size limit reached - rotating"));
    return true;
  }

  // Check if new day
  String newFileName = generateFileName(millis());
  if (newFileName != currentFileName) {
    Serial.println(F("[LOG] New day - rotating log file"));
    return true;
  }

  return false;
}

void DataLogger::writeHeader() {
  String header = "timestamp,temp_indoor,humidity_indoor,temp_outdoor,humidity_outdoor,pressure,light,iaq\n";
  logFile.print(header);
  Serial.println(F("[LOG] CSV header written"));
}

String DataLogger::generateFileName(unsigned long timestamp) {
  // Format: /logs/weather_YYYY_MM_DD.csv
  // For simplicity, using a generic name for now
  // TODO: Implement proper date extraction from RTC

  return String("/weather.csv");
}
