#pragma once

#include <Arduino.h>

/**
 * Mapa único de GPIOs do robô.
 *
 * Hardware previsto:
 * - ESP32-S3 DevKitC-1
 * - L298N com dois canais, cada canal acionando um lado do robô
 * - HC-SR04 frontal
 * - Sensor infravermelho na traseira
 */
namespace Pins {
  constexpr uint8_t MOTOR_LEFT_IN1 = 4;
  constexpr uint8_t MOTOR_LEFT_IN2 = 5;
  constexpr uint8_t MOTOR_RIGHT_IN1 = 6;
  constexpr uint8_t MOTOR_RIGHT_IN2 = 7;

  constexpr uint8_t MOTOR_LEFT_PWM = 15;
  constexpr uint8_t MOTOR_RIGHT_PWM = 16;

  constexpr uint8_t ULTRASONIC_TRIG = 17;
  constexpr uint8_t ULTRASONIC_ECHO = 18;

  constexpr uint8_t IR_SENSOR = 8;
}
