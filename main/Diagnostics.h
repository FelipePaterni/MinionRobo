#pragma once

#include <Arduino.h>

#include "RobotTypes.h"

/** Saída serial de diagnóstico com limitação temporal. */
class Diagnostics {
 public:
  void begin();
  void stateChanged(RobotState state, const char* reason);
  void status(
    RobotState state,
    const DistanceReading& distance,
    bool irDetected,
    int irRawLevel,
    MotorCommand command
  );
  void error(const char* message);

 private:
  uint32_t lastStatusAt_ = 0;
};

const char* robotStateName(RobotState state);
