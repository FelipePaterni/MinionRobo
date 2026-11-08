#pragma once

#include <Arduino.h>

enum class RobotState : uint8_t {
  Inicializando,
  Busca,
  VarreduraEsquerda,
  VarreduraCentro,
  VarreduraDireita,
  SeguindoAlvo,
  FugaParaFrente,
  GiroDeRecuperacao,
  ParadaSegura
};

struct DistanceReading {
  bool valid = false;
  float centimeters = -1.0f;
  uint32_t timestamp = 0;
};

struct MotorCommand {
  int16_t left = 0;
  int16_t right = 0;
};

struct ScanResult {
  DistanceReading left;
  DistanceReading center;
  DistanceReading right;
};
