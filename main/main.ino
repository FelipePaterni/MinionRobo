/**
 * MinionRobo - ponto de entrada do firmware.
 *
 * Este arquivo contém apenas a montagem dos módulos Arduino. Regras de
 * movimento, leitura de sensores e comportamento autônomo ficam nos arquivos
 * .cpp correspondentes para facilitar manutenção.
 */

#include "AutonomyController.h"
#include "Diagnostics.h"
#include "IrSensor.h"
#include "MotorController.h"
#include "UltrasonicSensor.h"
#include "WifiMonitor.h"

MotorController motores;
UltrasonicSensor ultrassonico;
IrSensor infravermelho;
Diagnostics diagnostico;

AutonomyController autonomia(
  motores,
  ultrassonico,
  infravermelho,
  diagnostico
);

WifiMonitor monitor(autonomia, motores);

void setup() {
  Serial.begin(115200);

  diagnostico.begin();
  motores.begin();
  ultrassonico.begin();
  infravermelho.begin();
  autonomia.begin();
  monitor.begin();
}

void loop() {
  autonomia.update();
  monitor.update();
}
