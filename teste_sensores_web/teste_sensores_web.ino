/*
 * MinionRobo - teste dos sensores com pagina web
 *
 * Ligações mantidas do firmware principal:
 *   HC-SR04 TRIG -> GPIO 17
 *   HC-SR04 ECHO -> GPIO 18
 *   Sensor IR traseiro -> GPIO 8
 *
 * Este sketch NÃO controla motores, ponte H ou direção.
 * Ele apenas lê os sensores e mostra a decisão que o robô tomaria.
 *
 * Importante: o ECHO do HC-SR04 não pode entrar no ESP32-S3 com 5 V.
 * Use divisor de tensão ou conversor de nível para reduzir para 3,3 V.
 */

#include <WiFi.h>
#include <WebServer.h>

constexpr uint8_t ULTRASONIC_TRIG = 17;
constexpr uint8_t ULTRASONIC_ECHO = 18;
constexpr uint8_t IR_SENSOR = 8;

// Mesma configuração do firmware principal.
constexpr bool IR_ACTIVE_LOW = true;
constexpr float DISTANCE_EMERGENCY_CM = 15.0f;
constexpr float DISTANCE_MINIMUM_CM = 28.0f;
constexpr float DISTANCE_MAX_TARGET_CM = 250.0f;

// Troque pelos dados da rede Wi-Fi usada pelo robô.
const char* WIFI_SSID = "**";
const char* WIFI_PASSWORD = "**";

WebServer server(80);

struct SensorStatus {
  bool ultrasonicValid = false;
  float distanceCm = -1.0f;
  int irRaw = HIGH;
  bool irObstacle = false;
  String action = "Inicializando";
};

SensorStatus status;
unsigned long lastReadMs = 0;

float readDistanceCm() {
  digitalWrite(ULTRASONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG, LOW);

  const unsigned long duration = pulseIn(ULTRASONIC_ECHO, HIGH, 30000UL);
  if (duration == 0) {
    return -1.0f;
  }

  const float distance = duration * 0.0343f / 2.0f;
  if (distance < 2.0f || distance > 400.0f) {
    return -1.0f;
  }
  return distance;
}

void calculateAction() {
  // A ordem segue a prioridade de segurança da máquina de estados atual.
  if (status.ultrasonicValid && status.distanceCm <= DISTANCE_EMERGENCY_CM) {
    status.action = "PARAR: obstaculo frontal critico";
  } else if (status.irObstacle) {
    status.action = "AVANCAR: obstaculo traseiro detectado";
  } else if (status.ultrasonicValid &&
             status.distanceCm <= DISTANCE_MINIMUM_CM) {
    status.action = "PARAR: objeto frontal muito proximo";
  } else if (status.ultrasonicValid &&
             status.distanceCm <= DISTANCE_MAX_TARGET_CM) {
    status.action = "SEGUIR ALVO: objeto detectado a frente";
  } else {
    status.action = "BUSCAR: nenhum alvo ultrassonico valido";
  }
}

void readSensors() {
  status.distanceCm = readDistanceCm();
  status.ultrasonicValid = status.distanceCm >= 0.0f;
  status.irRaw = digitalRead(IR_SENSOR);
  status.irObstacle = IR_ACTIVE_LOW ? status.irRaw == LOW : status.irRaw == HIGH;
  calculateAction();
}

String jsonStatus() {
  String json = "{";
  json += "\"ultrasonicValid\":" + String(status.ultrasonicValid ? "true" : "false");
  json += ",\"distanceCm\":";
  json += status.ultrasonicValid ? String(status.distanceCm, 1) : "null";
  json += ",\"irRaw\":" + String(status.irRaw);
  json += ",\"irObstacle\":" + String(status.irObstacle ? "true" : "false");
  json += ",\"action\":\"" + status.action + "\"";
  json += "}";
  return json;
}

const char PAGE[] PROGMEM = R"HTML(
<!doctype html><html lang="pt-BR"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>MinionRobo - teste dos sensores</title>
<style>
body{font-family:Arial,sans-serif;background:#111827;color:#f9fafb;max-width:760px;margin:30px auto;padding:0 18px}
h1{font-size:1.6rem}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:14px}
.card{background:#1f2937;border-radius:12px;padding:18px}.value{font-size:1.6rem;font-weight:bold;margin-top:8px}
.ok{color:#34d399}.warn{color:#fbbf24}.bad{color:#fb7185}.muted{color:#9ca3af}
#action{font-size:1.35rem;font-weight:bold;color:#60a5fa}.note{color:#9ca3af;font-size:.9rem}
</style></head><body>
<h1>Teste dos sensores — MinionRobo</h1>
<p class="note">Somente leitura. Motores e direção não são acionados.</p>
<div class="grid">
  <div class="card"><div>HC-SR04 · GPIO 17/18</div><div id="distance" class="value">--</div><div id="ultraState" class="muted">--</div></div>
  <div class="card"><div>IR traseiro · GPIO 8</div><div id="ir" class="value">--</div><div id="irRaw" class="muted">--</div></div>
  <div class="card"><div>O que o robô faria</div><div id="action">--</div></div>
</div>
<p class="note">Atualização automática a cada 500 ms.</p>
<script>
async function update(){
  try{
    const s=await (await fetch('/api/status',{cache:'no-store'})).json();
    distance.textContent=s.distanceCm===null?'Sem eco':s.distanceCm.toFixed(1)+' cm';
    ultraState.textContent=s.ultrasonicValid?'Leitura válida':'Verifique alimentação, TRIG/ECHO e divisor de tensão';
    ir.textContent=s.irObstacle?'OBSTÁCULO':'LIVRE';
    irRaw.textContent='Nível lógico bruto: '+s.irRaw;
    action.textContent=s.action;
    distance.className='value '+(s.ultrasonicValid?'ok':'bad');
    ir.className='value '+(s.irObstacle?'warn':'ok');
  }catch(e){action.textContent='Sem comunicação com a placa';}
}
setInterval(update,500); update();
</script></body></html>
)HTML";

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", PAGE);
}

void handleStatus() {
  server.send(200, "application/json; charset=utf-8", jsonStatus());
}

void setup() {
  Serial.begin(115200);
  pinMode(ULTRASONIC_TRIG, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);
  pinMode(IR_SENSOR, INPUT);
  digitalWrite(ULTRASONIC_TRIG, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.print("Abra no navegador: http://");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.begin();
  readSensors();
}

void loop() {
  server.handleClient();
  if (millis() - lastReadMs >= 250) {
    lastReadMs = millis();
    readSensors();
  }
}
