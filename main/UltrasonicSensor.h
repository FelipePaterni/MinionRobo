#pragma once

#include <Arduino.h>

#include "RobotTypes.h"

/**
 * Driver filtrado para o HC-SR04.
 * O ECHO deve chegar ao ESP32-S3 com no máximo 3,3 V.
 */
class UltrasonicSensor {
 public:
  void begin();
  DistanceReading read();

 private:
  static constexpr uint8_t HISTORY_SIZE = 3;
  float history_[HISTORY_SIZE] = {0.0f, 0.0f, 0.0f};
  uint8_t historyCount_ = 0;
  uint8_t historyIndex_ = 0;

  DistanceReading readRaw();
  void storeValidSample(float centimeters);
  float filteredDistance() const;
};
