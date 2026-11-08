#pragma once

#include <Arduino.h>

#include "RobotTypes.h"

/**
 * Encapsula o L298N e o PWM dos dois lados do robô.
 *
 * A classe recebe velocidades assinadas: positivo é avanço, negativo é ré.
 * A inversão física de cada lado fica configurada em Config.h.
 */
class MotorController {
 public:
  void begin();
  void update();

  void stop();
  void emergencyStop();
  void setSpeed(int left, int right);
  void forward(uint8_t pwm);
  void reverse(uint8_t pwm);
  void rotateLeft(uint8_t pwm);
  void rotateRight(uint8_t pwm);

  MotorCommand command() const;
  bool ready() const;

 private:
  int targetLeft_ = 0;
  int targetRight_ = 0;
  int appliedLeft_ = 0;
  int appliedRight_ = 0;
  uint32_t lastRampUpdate_ = 0;
  bool pwmReady_ = false;

  int stepToward(int current, int target) const;
  void applyMotor(uint8_t in1, uint8_t in2, uint8_t pwmPin, int speed);
};
