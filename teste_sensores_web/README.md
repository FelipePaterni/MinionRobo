# Teste dos sensores pela página web

Sketch para **ESP32-S3** que mantém a ligação do firmware principal e não aciona motores.

## Ligações

| Sensor | Pino ESP32-S3 | Observação |
|---|---:|---|
| HC-SR04 TRIG | GPIO 17 | saída |
| HC-SR04 ECHO | GPIO 18 | entrada; reduza 5 V para 3,3 V |
| IR traseiro, OUT | GPIO 8 | entrada; alimentação e GND comuns |

Também conecte GND dos sensores ao GND da ESP32. O HC-SR04 pode ser alimentado em 5 V, mas o sinal ECHO precisa passar por divisor de tensão ou conversor de nível antes do GPIO 18.

## Como usar

1. Abra `teste_sensores_web.ino` na Arduino IDE.
2. Selecione a placa ESP32-S3 e ajuste `WIFI_SSID` e `WIFI_PASSWORD`.
3. Grave o sketch com os motores desligados.
4. Abra o Monitor Serial em `115200` baud.
5. Acesse no navegador o endereço IP mostrado no Monitor Serial.

## Interpretação

- **HC-SR04: Sem eco**: verifique VCC, GND, TRIG, ECHO e o divisor de tensão.
- **IR: nível bruto 0/1**: cubra ou aproxime um obstáculo para confirmar a mudança. A configuração atual considera `LOW` como obstáculo, igual ao firmware principal.
- A caixa “O que o robô faria” é apenas uma simulação da decisão; nenhum GPIO de motor é usado.
