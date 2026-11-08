#include "IrSensor.h"

#include "Config.h"
#include "Pins.h"

void IrSensor::begin() {
  pinMode(Pins::IR_SENSOR, INPUT);
}

bool IrSensor::obstacleDetected() const {
  const int level = digitalRead(Pins::IR_SENSOR);
  return Config::IR_ACTIVE_LOW ? level == LOW : level == HIGH;
}

int IrSensor::rawLevel() const {
  return digitalRead(Pins::IR_SENSOR);
}
