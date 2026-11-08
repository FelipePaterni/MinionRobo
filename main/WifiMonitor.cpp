#include "WifiMonitor.h"

#include <WiFi.h>

#include "Diagnostics.h"

namespace {
  constexpr char WIFI_SSID[] = "MinionRobo";
  constexpr char WIFI_PASSWORD[] = "minion123";
}

WifiMonitor::WifiMonitor(
  const AutonomyController& autonomy,
  const MotorController& motors
)
  : autonomy_(autonomy), motors_(motors) {}

void WifiMonitor::begin() {
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println(F("ERRO: ponto de acesso Wi-Fi nao foi criado."));
    return;
  }

  Serial.print(F("Wi-Fi MinionRobo ativo em http://"));
  Serial.println(WiFi.softAPIP());

  server_.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
  server_.on("/api/health", HTTP_GET, [this]() { handleHealth(); });
  server_.on("/api/status", HTTP_OPTIONS, [this]() { handleOptions(); });
  server_.on("/api/health", HTTP_OPTIONS, [this]() { handleOptions(); });
  server_.onNotFound([this]() { handleNotFound(); });
  server_.begin();
}

void WifiMonitor::update() {
  server_.handleClient();
}

void WifiMonitor::handleStatus() {
  const DistanceReading& distance = autonomy_.distance();
  const MotorCommand command = autonomy_.motorCommand();

  String json = "{";
  json += "\"state\":\"";
  json += robotStateName(autonomy_.state());
  json += "\",\"distanceValid\":";
  json += distance.valid ? "true" : "false";
  json += ",\"distanceCm\":";
  json += distance.valid ? String(distance.centimeters, 1) : "null";
  json += ",\"irDetected\":";
  json += autonomy_.irDetected() ? "true" : "false";
  json += ",\"irRaw\":";
  json += String(autonomy_.irRawLevel());
  json += ",\"motorLeft\":";
  json += String(command.left);
  json += ",\"motorRight\":";
  json += String(command.right);
  json += ",\"motorsReady\":";
  json += motors_.ready() ? "true" : "false";
  json += "}";

  sendJson(200, json);
}

void WifiMonitor::handleHealth() {
  String json = "{\"ok\":true,\"uptimeMs\":";
  json += String(millis());
  json += ",\"ip\":\"";
  json += WiFi.softAPIP().toString();
  json += "\"}";
  sendJson(200, json);
}

void WifiMonitor::handleOptions() {
  server_.sendHeader("Access-Control-Allow-Origin", "*");
  server_.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server_.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server_.send(204, "text/plain", "");
}

void WifiMonitor::handleNotFound() {
  sendJson(404, "{\"error\":\"endpoint not found\"}");
}

void WifiMonitor::sendJson(int statusCode, const String& body) {
  server_.sendHeader("Access-Control-Allow-Origin", "*");
  server_.sendHeader("Cache-Control", "no-store");
  server_.send(statusCode, "application/json; charset=utf-8", body);
}
