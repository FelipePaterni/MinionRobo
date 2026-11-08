#pragma once

#include <Arduino.h>

/** Abstrai o sensor infravermelho traseiro. */
class IrSensor {
 public:
  void begin();
  bool obstacleDetected() const;
  int rawLevel() const;
};
