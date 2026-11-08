#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "AutonomyController.h"
#include "MotorController.h"

/** Ponto de acesso e API somente leitura para monitorar a autonomia. */
class WifiMonitor {
 public:
  explicit WifiMonitor(const AutonomyController& autonomy, const MotorController& motors);

  void begin();
  void update();

 private:
  const AutonomyController& autonomy_;
  const MotorController& motors_;
  WebServer server_{80};

  void handleStatus();
  void handleHealth();
  void handleOptions();
  void handleNotFound();
  void sendJson(int statusCode, const String& body);
};
