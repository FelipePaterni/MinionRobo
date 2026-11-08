#include "MotorController.h"

#include "Config.h"
#include "Pins.h"

void MotorController::begin() {
  pinMode(Pins::MOTOR_LEFT_IN1, OUTPUT);
  pinMode(Pins::MOTOR_LEFT_IN2, OUTPUT);
  pinMode(Pins::MOTOR_RIGHT_IN1, OUTPUT);
  pinMode(Pins::MOTOR_RIGHT_IN2, OUTPUT);

  const bool leftAttached = ledcAttach(
    Pins::MOTOR_LEFT_PWM,
    Config::PWM_FREQUENCY,
    Config::PWM_RESOLUTION
  );
  const bool rightAttached = ledcAttach(
    Pins::MOTOR_RIGHT_PWM,
    Config::PWM_FREQUENCY,
    Config::PWM_RESOLUTION
  );

  pwmReady_ = leftAttached && rightAttached;
  if (!leftAttached || !rightAttached) {
    emergencyStop();
    Serial.println(F("ERRO: PWM dos motores nao configurado."));
    return;
  }

  emergencyStop();
}

void MotorController::update() {
  if (!pwmReady_) {
    emergencyStop();
    return;
  }

  const uint32_t now = millis();
  if (now - lastRampUpdate_ < 10) {
    return;
  }
  lastRampUpdate_ = now;

  appliedLeft_ = stepToward(appliedLeft_, targetLeft_);
  appliedRight_ = stepToward(appliedRight_, targetRight_);

  applyMotor(
    Pins::MOTOR_LEFT_IN1,
    Pins::MOTOR_LEFT_IN2,
    Pins::MOTOR_LEFT_PWM,
    Config::INVERT_LEFT_MOTOR ? -appliedLeft_ : appliedLeft_
  );
  applyMotor(
    Pins::MOTOR_RIGHT_IN1,
    Pins::MOTOR_RIGHT_IN2,
    Pins::MOTOR_RIGHT_PWM,
    Config::INVERT_RIGHT_MOTOR ? -appliedRight_ : appliedRight_
  );
}

void MotorController::stop() {
  setSpeed(0, 0);
}

void MotorController::emergencyStop() {
  targetLeft_ = 0;
  targetRight_ = 0;
  appliedLeft_ = 0;
  appliedRight_ = 0;

  ledcWrite(Pins::MOTOR_LEFT_PWM, 0);
  ledcWrite(Pins::MOTOR_RIGHT_PWM, 0);

  digitalWrite(Pins::MOTOR_LEFT_IN1, LOW);
  digitalWrite(Pins::MOTOR_LEFT_IN2, LOW);
  digitalWrite(Pins::MOTOR_RIGHT_IN1, LOW);
  digitalWrite(Pins::MOTOR_RIGHT_IN2, LOW);
}

void MotorController::setSpeed(int left, int right) {
  targetLeft_ = constrain(left, -Config::PWM_MAX, Config::PWM_MAX);
  targetRight_ = constrain(right, -Config::PWM_MAX, Config::PWM_MAX);
}

void MotorController::forward(uint8_t pwm) {
  setSpeed(pwm, pwm);
}

void MotorController::reverse(uint8_t pwm) {
  setSpeed(-static_cast<int>(pwm), -static_cast<int>(pwm));
}

void MotorController::rotateLeft(uint8_t pwm) {
  setSpeed(-static_cast<int>(pwm), pwm);
}

void MotorController::rotateRight(uint8_t pwm) {
  setSpeed(pwm, -static_cast<int>(pwm));
}

MotorCommand MotorController::command() const {
  MotorCommand result;
  result.left = static_cast<int16_t>(targetLeft_);
  result.right = static_cast<int16_t>(targetRight_);
  return result;
}

bool MotorController::ready() const {
  return pwmReady_;
}

int MotorController::stepToward(int current, int target) const {
  if (current == target) {
    return current;
  }

  constexpr int STEP = 10;
  if (current < target) {
    return min(current + STEP, target);
  }
  return max(current - STEP, target);
}

void MotorController::applyMotor(
  uint8_t in1,
  uint8_t in2,
  uint8_t pwmPin,
  int speed
) {
  speed = constrain(speed, -Config::PWM_MAX, Config::PWM_MAX);

  if (speed == 0) {
    ledcWrite(pwmPin, 0);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    return;
  }

  const int absoluteSpeed = abs(speed);
  const int pwm = max(absoluteSpeed, Config::PWM_START_MINIMUM);

  digitalWrite(in1, speed > 0 ? HIGH : LOW);
  digitalWrite(in2, speed < 0 ? HIGH : LOW);
  ledcWrite(pwmPin, pwm);
}
