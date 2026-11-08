#pragma once

#include <Arduino.h>

#include "Diagnostics.h"
#include "IrSensor.h"
#include "MotorController.h"
#include "RobotTypes.h"
#include "UltrasonicSensor.h"

/**
 * Máquina de estados do robô.
 * O controlador não acessa GPIO diretamente.
 */
class AutonomyController {
 public:
  AutonomyController(
    MotorController& motors,
    UltrasonicSensor& ultrasonic,
    IrSensor& ir,
    Diagnostics& diagnostics
  );

  void begin();
  void update();

  RobotState state() const;
  const DistanceReading& distance() const;
  bool irDetected() const;
  int irRawLevel() const;
  MotorCommand motorCommand() const;

 private:
  MotorController& motors_;
  UltrasonicSensor& ultrasonic_;
  IrSensor& ir_;
  Diagnostics& diagnostics_;

  RobotState state_ = RobotState::Inicializando;
  RobotState resumeState_ = RobotState::Busca;
  RobotState scanReturnState_ = RobotState::Busca;

  DistanceReading distance_;
  ScanResult scan_;

  uint32_t stateStartedAt_ = 0;
  uint32_t lastSensorReadAt_ = 0;
  uint32_t lastTargetSeenAt_ = 0;
  uint32_t targetStableSince_ = 0;
  uint32_t lastSearchChangeAt_ = 0;
  uint32_t searchSegmentDuration_ = 0;
  uint32_t nextScanAt_ = 0;
  float lastTargetDistance_ = -1.0f;
  int8_t searchDirection_ = 0;
  bool scanCorrectionActive_ = false;
  int8_t scanCorrectionDirection_ = 0;
  uint32_t scanCorrectionUntil_ = 0;

  void readSensors(uint32_t now);
  void updateState(uint32_t now);
  void updateInitialization(uint32_t now);
  void updateSearch(uint32_t now);
  void updateScanLeft(uint32_t now);
  void updateScanCenter(uint32_t now);
  void updateScanRight(uint32_t now);
  void updateFollow(uint32_t now);
  void updateEscape(uint32_t now);
  void updateRecovery(uint32_t now);
  void updateSafeStop(uint32_t now);

  void enterState(RobotState state, const char* reason);
  void startScan(RobotState returnState, uint32_t now);
  void finishScan(uint32_t now);
  void handleGlobalSafety();
  bool hasTarget() const;
  bool targetIsStable(uint32_t now);
  bool shouldScan(uint32_t now) const;
  void chooseSearchMovement(uint32_t now);
  void applyScanCorrection(uint32_t now);
  void resetTargetTracking();
};
