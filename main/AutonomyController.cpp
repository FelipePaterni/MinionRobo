#include "AutonomyController.h"

#include <math.h>

#include "Config.h"

AutonomyController::AutonomyController(
  MotorController& motors,
  UltrasonicSensor& ultrasonic,
  IrSensor& ir,
  Diagnostics& diagnostics
)
  : motors_(motors),
    ultrasonic_(ultrasonic),
    ir_(ir),
    diagnostics_(diagnostics) {}

void AutonomyController::begin() {
  randomSeed(static_cast<uint32_t>(micros()));
  stateStartedAt_ = millis();
  nextScanAt_ = stateStartedAt_ + Config::SEARCH_SCAN_INTERVAL;
  motors_.emergencyStop();
  diagnostics_.stateChanged(state_, "Inicializacao segura");
}

void AutonomyController::update() {
  const uint32_t now = millis();

  readSensors(now);
  handleGlobalSafety();
  updateState(now);
  motors_.update();

  diagnostics_.status(
    state_,
    distance_,
    ir_.obstacleDetected(),
    ir_.rawLevel(),
    motors_.command()
  );
}

RobotState AutonomyController::state() const {
  return state_;
}

const DistanceReading& AutonomyController::distance() const {
  return distance_;
}

bool AutonomyController::irDetected() const {
  return ir_.obstacleDetected();
}

int AutonomyController::irRawLevel() const {
  return ir_.rawLevel();
}

MotorCommand AutonomyController::motorCommand() const {
  return motors_.command();
}

void AutonomyController::readSensors(uint32_t now) {
  if (now - lastSensorReadAt_ < Config::SENSOR_READ_INTERVAL) {
    return;
  }

  lastSensorReadAt_ = now;
  distance_ = ultrasonic_.read();

  if (distance_.valid &&
      distance_.centimeters <= Config::MAX_TARGET_DISTANCE) {
    lastTargetSeenAt_ = now;
  }
}

void AutonomyController::updateState(uint32_t now) {
  switch (state_) {
    case RobotState::Inicializando:
      updateInitialization(now);
      break;
    case RobotState::Busca:
      updateSearch(now);
      break;
    case RobotState::VarreduraEsquerda:
      updateScanLeft(now);
      break;
    case RobotState::VarreduraCentro:
      updateScanCenter(now);
      break;
    case RobotState::VarreduraDireita:
      updateScanRight(now);
      break;
    case RobotState::SeguindoAlvo:
      updateFollow(now);
      break;
    case RobotState::FugaParaFrente:
      updateEscape(now);
      break;
    case RobotState::GiroDeRecuperacao:
      updateRecovery(now);
      break;
    case RobotState::ParadaSegura:
      updateSafeStop(now);
      break;
  }
}

void AutonomyController::updateInitialization(uint32_t now) {
  motors_.stop();
  if (now - stateStartedAt_ >= Config::INITIALIZATION_TIME) {
    enterState(RobotState::Busca, "Inicializacao concluida");
  }
}

void AutonomyController::updateSearch(uint32_t now) {
  if (scanCorrectionActive_) {
    applyScanCorrection(now);
    if (scanCorrectionActive_) {
      return;
    }
  }

  if (hasTarget()) {
    enterState(RobotState::SeguindoAlvo, "Alvo detectado durante busca");
    return;
  }

  if (shouldScan(now)) {
    startScan(RobotState::Busca, now);
    return;
  }

  if (now - lastSearchChangeAt_ >= searchSegmentDuration_) {
    chooseSearchMovement(now);
  }

  if (searchDirection_ < 0) {
    motors_.rotateLeft(Config::PWM_SEARCH);
  } else if (searchDirection_ > 0) {
    motors_.rotateRight(Config::PWM_SEARCH);
  } else {
    motors_.forward(Config::PWM_SEARCH);
  }
}

void AutonomyController::updateScanLeft(uint32_t now) {
  if (now - stateStartedAt_ < Config::SCAN_STEP_TIME) {
    motors_.rotateLeft(Config::PWM_SCAN);
    return;
  }

  motors_.stop();
  scan_.left = distance_;
  enterState(RobotState::VarreduraCentro, "Leitura esquerda concluida");
}

void AutonomyController::updateScanCenter(uint32_t now) {
  // Volta da posição esquerda para o centro antes de medir.
  if (now - stateStartedAt_ < Config::SCAN_STEP_TIME) {
    motors_.rotateRight(Config::PWM_SCAN);
    return;
  }

  motors_.stop();
  scan_.center = distance_;
  enterState(RobotState::VarreduraDireita, "Leitura central concluida");
}

void AutonomyController::updateScanRight(uint32_t now) {
  // Da posição central até a posição direita são dois passos de varredura.
  if (now - stateStartedAt_ < Config::SCAN_STEP_TIME * 2UL) {
    motors_.rotateRight(Config::PWM_SCAN);
    return;
  }

  motors_.stop();
  scan_.right = distance_;
  finishScan(now);
}

void AutonomyController::updateFollow(uint32_t now) {
  if (scanCorrectionActive_) {
    applyScanCorrection(now);
    if (scanCorrectionActive_) {
      return;
    }
  }

  if (!hasTarget()) {
    motors_.stop();
    if (now - lastTargetSeenAt_ >= Config::LOST_TARGET_TIME) {
      resetTargetTracking();
      enterState(RobotState::Busca, "Alvo perdido");
    }
    return;
  }

  if (targetIsStable(now)) {
    motors_.stop();
    if (now - targetStableSince_ >= Config::TARGET_STATIONARY_TIME) {
      enterState(RobotState::GiroDeRecuperacao, "Alvo parado por muito tempo");
      return;
    }
  }

  if (distance_.centimeters <= Config::DISTANCE_MINIMUM) {
    motors_.stop();
  } else if (distance_.centimeters >
             Config::DISTANCE_DESIRED + Config::DISTANCE_TOLERANCE) {
    motors_.forward(Config::PWM_FOLLOW);
  } else if (distance_.centimeters <
             Config::DISTANCE_DESIRED - Config::DISTANCE_TOLERANCE) {
    motors_.stop();
  } else {
    motors_.forward(Config::PWM_FOLLOW_SLOW);
  }

  if (shouldScan(now)) {
    startScan(RobotState::SeguindoAlvo, now);
  }
}

void AutonomyController::updateEscape(uint32_t now) {
  const bool rearObstacle = ir_.obstacleDetected();
  const bool frontTooClose =
    distance_.valid && distance_.centimeters <= Config::DISTANCE_MINIMUM;

  // O IR liberado encerra a fuga antes de avaliar a distância frontal.
  // Isso evita ficar parado em FugaParaFrente quando o robô já está livre
  // atrás, mas encontrou uma parede à frente.
  if (!rearObstacle) {
    motors_.stop();
    if (frontTooClose) {
      enterState(
        RobotState::GiroDeRecuperacao,
        "Fuga concluida, obstaculo frontal impede avancar"
      );
    } else {
      enterState(resumeState_, "Obstaculo traseiro deixou de ser detectado");
    }
    return;
  }

  // Enquanto ainda há obstáculo atrás, só avance se houver espaço frontal.
  // A saída por tempo impede um estado de fuga infinito em caso de leitura
  // presa ou de um obstáculo que bloqueie os dois lados.
  if (frontTooClose) {
    motors_.stop();
  } else {
    motors_.forward(Config::PWM_ESCAPE);
  }

  if (now - stateStartedAt_ >= Config::ESCAPE_MAX_TIME) {
    motors_.stop();
    enterState(
      RobotState::GiroDeRecuperacao,
      "Tempo maximo de fuga atingido"
    );
  }
}

void AutonomyController::updateRecovery(uint32_t now) {
  const uint32_t elapsed = now - stateStartedAt_;

  if (elapsed < Config::RECOVERY_STOP_TIME) {
    motors_.stop();
    return;
  }

  if (elapsed < Config::RECOVERY_STOP_TIME + Config::TURN_180_TIME) {
    motors_.rotateLeft(Config::PWM_RECOVERY_TURN);
    return;
  }

  motors_.stop();
  resetTargetTracking();
  enterState(RobotState::Busca, "Giro de recuperacao concluido");
}

void AutonomyController::updateSafeStop(uint32_t now) {
  motors_.emergencyStop();

  const bool obstacleStillClose =
    distance_.valid && distance_.centimeters <= Config::DISTANCE_EMERGENCY;

  if (!obstacleStillClose &&
      now - stateStartedAt_ >= Config::SAFE_STOP_HOLD_TIME) {
    enterState(resumeState_, "Area frontal liberada");
    return;
  }

  if (now - stateStartedAt_ >= Config::SAFE_STOP_MAX_TIME) {
    enterState(RobotState::GiroDeRecuperacao, "Obstaculo frontal persistente");
  }
}

void AutonomyController::enterState(
  RobotState state,
  const char* reason
) {
  state_ = state;
  stateStartedAt_ = millis();
  diagnostics_.stateChanged(state_, reason);

  if (state_ == RobotState::Busca) {
    lastSearchChangeAt_ = stateStartedAt_;
    searchSegmentDuration_ = Config::SEARCH_SEGMENT_MIN;
    nextScanAt_ = stateStartedAt_ + Config::SEARCH_SCAN_INTERVAL;
    chooseSearchMovement(stateStartedAt_);
  }

  if (state_ == RobotState::SeguindoAlvo) {
    nextScanAt_ = stateStartedAt_ + Config::FOLLOW_SCAN_INTERVAL;
  }
}

void AutonomyController::startScan(RobotState returnState, uint32_t now) {
  scanReturnState_ = returnState;
  scan_.center = distance_;
  scan_.left = DistanceReading();
  scan_.right = DistanceReading();
  nextScanAt_ = now + Config::FOLLOW_SCAN_INTERVAL;
  enterState(RobotState::VarreduraEsquerda, "Iniciando varredura");
}

void AutonomyController::finishScan(uint32_t now) {
  const bool leftValid = scan_.left.valid;
  const bool centerValid = scan_.center.valid;
  const bool rightValid = scan_.right.valid;

  float bestDistance = 100000.0f;
  int8_t bestDirection = 0;

  if (leftValid && scan_.left.centimeters < bestDistance) {
    bestDistance = scan_.left.centimeters;
    bestDirection = -1;
  }
  if (centerValid && scan_.center.centimeters < bestDistance) {
    bestDistance = scan_.center.centimeters;
    bestDirection = 0;
  }
  if (rightValid && scan_.right.centimeters < bestDistance) {
    bestDistance = scan_.right.centimeters;
    bestDirection = 1;
  }

  if (bestDirection == 0 && centerValid) {
    enterState(scanReturnState_, "Varredura manteve direcao central");
    return;
  }

  if (bestDirection < 0) {
    scanCorrectionDirection_ = -1;
  } else if (bestDirection > 0) {
    scanCorrectionDirection_ = 1;
  } else {
    enterState(RobotState::Busca, "Nenhum alvo encontrado na varredura");
    return;
  }

  scanCorrectionActive_ = true;
  scanCorrectionUntil_ = now + Config::SCAN_CORRECTION_TIME;
  enterState(scanReturnState_, "Direcao escolhida pela varredura");
}

void AutonomyController::handleGlobalSafety() {
  if (state_ == RobotState::Inicializando ||
      state_ == RobotState::GiroDeRecuperacao ||
      state_ == RobotState::ParadaSegura) {
    return;
  }

  if (distance_.valid &&
      distance_.centimeters <= Config::DISTANCE_EMERGENCY) {
    resumeState_ = state_;
    enterState(RobotState::ParadaSegura, "Obstaculo frontal critico");
    return;
  }

  if (Config::STOP_ON_INVALID_ULTRASONIC &&
      !distance_.valid &&
      state_ != RobotState::Busca) {
    resumeState_ = state_;
    enterState(RobotState::ParadaSegura, "Leitura ultrassonica invalida");
    return;
  }

  if (ir_.obstacleDetected() && state_ != RobotState::FugaParaFrente) {
    resumeState_ = state_;
    enterState(RobotState::FugaParaFrente, "Obstaculo traseiro detectado");
  }
}

bool AutonomyController::hasTarget() const {
  return distance_.valid &&
         distance_.centimeters <= Config::MAX_TARGET_DISTANCE;
}

bool AutonomyController::targetIsStable(uint32_t now) {
  if (!distance_.valid) {
    return false;
  }

  if (lastTargetDistance_ < 0.0f ||
      fabsf(distance_.centimeters - lastTargetDistance_) >
        Config::TARGET_STATIONARY_TOLERANCE) {
    lastTargetDistance_ = distance_.centimeters;
    targetStableSince_ = now;
    return false;
  }

  if (targetStableSince_ == 0) {
    targetStableSince_ = now;
  }

  return true;
}

bool AutonomyController::shouldScan(uint32_t now) const {
  return now >= nextScanAt_ &&
         (state_ == RobotState::Busca || state_ == RobotState::SeguindoAlvo);
}

void AutonomyController::chooseSearchMovement(uint32_t now) {
  const long choice = random(0, 3);
  if (choice == 0) {
    searchDirection_ = 0;
  } else if (choice == 1) {
    searchDirection_ = -1;
  } else {
    searchDirection_ = 1;
  }

  searchSegmentDuration_ = static_cast<uint32_t>(random(
    static_cast<long>(Config::SEARCH_SEGMENT_MIN),
    static_cast<long>(Config::SEARCH_SEGMENT_MAX) + 1L
  ));
  lastSearchChangeAt_ = now;
}

void AutonomyController::applyScanCorrection(uint32_t now) {
  if (now >= scanCorrectionUntil_) {
    scanCorrectionActive_ = false;
    scanCorrectionDirection_ = 0;
    motors_.stop();
    return;
  }

  if (scanCorrectionDirection_ < 0) {
    motors_.rotateLeft(Config::PWM_SCAN);
  } else {
    motors_.rotateRight(Config::PWM_SCAN);
  }
}

void AutonomyController::resetTargetTracking() {
  lastTargetDistance_ = -1.0f;
  targetStableSince_ = 0;
}
