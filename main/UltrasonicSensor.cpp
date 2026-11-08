#include "UltrasonicSensor.h"

#include "Pins.h"

namespace {
  constexpr uint32_t ECHO_TIMEOUT_MICROS = 30000;
  constexpr float SOUND_SPEED_CM_PER_MICROSECOND = 0.0343f;
  constexpr float MIN_DISTANCE_CM = 2.0f;
  constexpr float MAX_DISTANCE_CM = 400.0f;
}

void UltrasonicSensor::begin() {
  pinMode(Pins::ULTRASONIC_TRIG, OUTPUT);
  pinMode(Pins::ULTRASONIC_ECHO, INPUT);
  digitalWrite(Pins::ULTRASONIC_TRIG, LOW);
}

DistanceReading UltrasonicSensor::read() {
  DistanceReading result = readRaw();

  if (!result.valid) {
    return result;
  }

  storeValidSample(result.centimeters);
  result.centimeters = filteredDistance();
  result.timestamp = millis();
  return result;
}

DistanceReading UltrasonicSensor::readRaw() {
  DistanceReading result;

  digitalWrite(Pins::ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(Pins::ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(Pins::ULTRASONIC_TRIG, LOW);

  const uint32_t duration = pulseIn(
    Pins::ULTRASONIC_ECHO,
    HIGH,
    ECHO_TIMEOUT_MICROS
  );

  if (duration == 0) {
    return result;
  }

  const float centimeters =
    duration * SOUND_SPEED_CM_PER_MICROSECOND / 2.0f;

  if (centimeters < MIN_DISTANCE_CM || centimeters > MAX_DISTANCE_CM) {
    return result;
  }

  result.valid = true;
  result.centimeters = centimeters;
  result.timestamp = millis();
  return result;
}

void UltrasonicSensor::storeValidSample(float centimeters) {
  history_[historyIndex_] = centimeters;
  historyIndex_ = (historyIndex_ + 1) % HISTORY_SIZE;
  if (historyCount_ < HISTORY_SIZE) {
    ++historyCount_;
  }
}

float UltrasonicSensor::filteredDistance() const {
  if (historyCount_ == 0) {
    return -1.0f;
  }

  float values[HISTORY_SIZE];
  for (uint8_t i = 0; i < historyCount_; ++i) {
    values[i] = history_[i];
  }

  for (uint8_t i = 0; i < historyCount_; ++i) {
    for (uint8_t j = i + 1; j < historyCount_; ++j) {
      if (values[j] < values[i]) {
        const float temporary = values[i];
        values[i] = values[j];
        values[j] = temporary;
      }
    }
  }

  return values[historyCount_ / 2];
}
