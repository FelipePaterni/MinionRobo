/*
  MinionRobo - controle remoto por Wi-Fi

  O ESP32-S3 cria uma rede Wi-Fi propria. Conecte o celular nessa rede e
  abra http://192.168.4.1 para controlar os motores.

  Ligacao mantida do teste original:
  - Motor esquerdo: IN1 GPIO 4, IN2 GPIO 5, PWM/ENA GPIO 15
  - Motor direito:  IN1 GPIO 6, IN2 GPIO 7, PWM/ENB GPIO 16
  - LED RGB integrado: GPIO 48
*/

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

constexpr uint8_t MOTOR_LEFT_IN1  = 4;
constexpr uint8_t MOTOR_LEFT_IN2  = 5;
constexpr uint8_t MOTOR_RIGHT_IN1 = 6;
constexpr uint8_t MOTOR_RIGHT_IN2 = 7;
constexpr uint8_t MOTOR_LEFT_PWM  = 15;
constexpr uint8_t MOTOR_RIGHT_PWM = 16;
constexpr uint8_t RGB_LED_PIN     = 48;

constexpr uint32_t PWM_FREQUENCY = 1000;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint8_t VELOCIDADE = 160;

constexpr char WIFI_NOME[] = "MinionRobo";
constexpr char WIFI_SENHA[] = "minion123";
constexpr uint32_t TIMEOUT_COMANDO = 800;

WebServer server(80);
uint32_t ultimoComando = 0;
bool motoresEmMovimento = false;

const char PAGINA_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
  <title>MinionRobo</title>
  <style>
    * { box-sizing: border-box; }
    body { margin: 0; min-height: 100vh; display: grid; place-items: center;
      background: #10131a; color: white; font-family: sans-serif; }
    main { width: min(94vw, 420px); text-align: center; }
    h1 { margin: 0 0 8px; }
    p { color: #aeb6c5; margin: 0 0 22px; }
    .controle { display: grid; grid-template-columns: repeat(3, 1fr); gap: 12px; }
    button { min-height: 86px; border: 0; border-radius: 18px; color: white;
      font-size: 19px; font-weight: bold; touch-action: none; user-select: none;
      -webkit-user-select: none; }
    button:active { filter: brightness(1.35); transform: scale(.97); }
    .direcao { background: #1769aa; }
    .giro { background: #7b3fb6; }
    .parar { background: #b52635; }
    .vazio { visibility: hidden; }
  </style>
</head>
<body>
  <main>
    <h1>MinionRobo</h1>
    <p>Pressione e mantenha o botao</p>
    <section class="controle">
      <button class="vazio" tabindex="-1"></button>
      <button class="direcao" data-mov="frente">▲<br>FRENTE</button>
      <button class="vazio" tabindex="-1"></button>
      <button class="giro" data-mov="esquerda">◀<br>ESQUERDA</button>
      <button class="parar" data-mov="parar">■<br>PARAR</button>
      <button class="giro" data-mov="direita">▶<br>DIREITA</button>
      <button class="vazio" tabindex="-1"></button>
      <button class="direcao" data-mov="re">▼<br>RE</button>
      <button class="vazio" tabindex="-1"></button>
    </section>
  </main>
  <script>
    let repeticao = null;
    function enviar(mov) {
      fetch('/cmd?mov=' + mov, {cache: 'no-store'}).catch(() => {});
    }
    function iniciar(mov) {
      if (mov === 'parar') { parar(); return; }
      parar();
      enviar(mov);
      repeticao = setInterval(() => enviar(mov), 200);
    }
    function parar() {
      if (repeticao) clearInterval(repeticao);
      repeticao = null;
      enviar('parar');
    }
    document.querySelectorAll('[data-mov]').forEach(botao => {
      botao.addEventListener('pointerdown', e => {
        e.preventDefault();
        botao.setPointerCapture(e.pointerId);
        iniciar(botao.dataset.mov);
      });
      botao.addEventListener('pointerup', e => { e.preventDefault(); parar(); });
      botao.addEventListener('pointercancel', parar);
      botao.addEventListener('contextmenu', e => e.preventDefault());
    });
    window.addEventListener('blur', parar);
  </script>
</body>
</html>
)rawliteral";

void ledCor(uint8_t vermelho, uint8_t verde, uint8_t azul) {
  neopixelWrite(RGB_LED_PIN, vermelho, verde, azul);
}

void motor(uint8_t in1, uint8_t in2, uint8_t pwmPin, int velocidade) {
  velocidade = constrain(velocidade, -255, 255);
  if (velocidade == 0) {
    ledcWrite(pwmPin, 0);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    return;
  }
  digitalWrite(in1, velocidade > 0 ? HIGH : LOW);
  digitalWrite(in2, velocidade < 0 ? HIGH : LOW);
  ledcWrite(pwmPin, abs(velocidade));
}

void pararMotores() {
  motor(MOTOR_LEFT_IN1, MOTOR_LEFT_IN2, MOTOR_LEFT_PWM, 0);
  motor(MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2, MOTOR_RIGHT_PWM, 0);
  ledCor(0, 0, 0);
  motoresEmMovimento = false;
}

void aplicarMovimento(const String& movimento) {
  ultimoComando = millis();

  if (movimento == "frente") {
    motor(MOTOR_LEFT_IN1, MOTOR_LEFT_IN2, MOTOR_LEFT_PWM, +VELOCIDADE);
    motor(MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2, MOTOR_RIGHT_PWM, +VELOCIDADE);
    ledCor(0, 255, 0);
  } else if (movimento == "re") {
    motor(MOTOR_LEFT_IN1, MOTOR_LEFT_IN2, MOTOR_LEFT_PWM, -VELOCIDADE);
    motor(MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2, MOTOR_RIGHT_PWM, -VELOCIDADE);
    ledCor(0, 0, 255);
  } else if (movimento == "direita") {
    motor(MOTOR_LEFT_IN1, MOTOR_LEFT_IN2, MOTOR_LEFT_PWM, +VELOCIDADE);
    motor(MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2, MOTOR_RIGHT_PWM, -VELOCIDADE);
    ledCor(255, 180, 0);
  } else if (movimento == "esquerda") {
    motor(MOTOR_LEFT_IN1, MOTOR_LEFT_IN2, MOTOR_LEFT_PWM, -VELOCIDADE);
    motor(MOTOR_RIGHT_IN1, MOTOR_RIGHT_IN2, MOTOR_RIGHT_PWM, +VELOCIDADE);
    ledCor(180, 0, 255);
  } else {
    pararMotores();
    return;
  }

  motoresEmMovimento = true;
}

void paginaInicial() {
  server.send(200, "text/html; charset=utf-8", PAGINA_HTML);
}

void comandoMotor() {
  if (!server.hasArg("mov")) {
    server.send(400, "text/plain", "Parametro mov ausente");
    return;
  }
  aplicarMovimento(server.arg("mov"));
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_LEFT_IN1, OUTPUT);
  pinMode(MOTOR_LEFT_IN2, OUTPUT);
  pinMode(MOTOR_RIGHT_IN1, OUTPUT);
  pinMode(MOTOR_RIGHT_IN2, OUTPUT);
  pinMode(RGB_LED_PIN, OUTPUT);

  ledcAttach(MOTOR_LEFT_PWM, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(MOTOR_RIGHT_PWM, PWM_FREQUENCY, PWM_RESOLUTION);
  pararMotores();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_NOME, WIFI_SENHA);

  Serial.println();
  Serial.println("Rede criada:");
  Serial.print("  Nome: ");
  Serial.println(WIFI_NOME);
  Serial.print("  Senha: ");
  Serial.println(WIFI_SENHA);
  Serial.print("  Pagina: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, paginaInicial);
  server.on("/cmd", HTTP_GET, comandoMotor);
  server.onNotFound([]() {
    server.send(404, "text/plain", "Nao encontrado");
  });
  server.begin();
  ultimoComando = millis();
}

void loop() {
  server.handleClient();

  // Para se o celular perder a conexao ou o botao deixar de ser pressionado.
  if (motoresEmMovimento && millis() - ultimoComando > TIMEOUT_COMANDO) {
    pararMotores();
  }
}
