#include "Diagnostics.h"

#include "Config.h"

void Diagnostics::begin() {
  if (!Config::ENABLE_DIAGNOSTICS) {
    return;
  }
  Serial.println(F("MinionRobo: diagnostico iniciado."));
}

void Diagnostics::stateChanged(RobotState state, const char* reason) {
  if (!Config::ENABLE_DIAGNOSTICS) {
    return;
  }

  Serial.print(F("Estado: "));
  Serial.print(robotStateName(state));
  Serial.print(F(" | Motivo: "));
  Serial.println(reason);
}

void Diagnostics::status(
  RobotState state,
  const DistanceReading& distance,
  bool irDetected,
  int irRawLevel,
  MotorCommand command
) {
  if (!Config::ENABLE_DIAGNOSTICS) {
    return;
  }

  const uint32_t now = millis();
  if (now - lastStatusAt_ < Config::DIAGNOSTIC_INTERVAL) {
    return;
  }
  lastStatusAt_ = now;

  Serial.print(F("Status | estado="));
  Serial.print(robotStateName(state));
  Serial.print(F(" | distancia="));
  if (distance.valid) {
    Serial.print(distance.centimeters, 1);
    Serial.print(F("cm"));
  } else {
    Serial.print(F("sem-eco"));
  }
  Serial.print(F(" | IR="));
  Serial.print(irDetected ? F("DETECTADO") : F("livre"));
  Serial.print(F("/raw="));
  Serial.print(irRawLevel);
  Serial.print(F(" | motorL="));
  Serial.print(command.left);
  Serial.print(F(" | motorR="));
  Serial.println(command.right);
}

void Diagnostics::error(const char* message) {
  if (!Config::ENABLE_DIAGNOSTICS) {
    return;
  }
  Serial.print(F("ERRO: "));
  Serial.println(message);
}

const char* robotStateName(RobotState state) {
  switch (state) {
    case RobotState::Inicializando: return "Inicializando";
    case RobotState::Busca: return "Busca";
    case RobotState::VarreduraEsquerda: return "VarreduraEsquerda";
    case RobotState::VarreduraCentro: return "VarreduraCentro";
    case RobotState::VarreduraDireita: return "VarreduraDireita";
    case RobotState::SeguindoAlvo: return "SeguindoAlvo";
    case RobotState::FugaParaFrente: return "FugaParaFrente";
    case RobotState::GiroDeRecuperacao: return "GiroDeRecuperacao";
    case RobotState::ParadaSegura: return "ParadaSegura";
  }
  return "Desconhecido";
}
