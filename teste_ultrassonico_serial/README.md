# Teste isolado do sensor ultrassônico

Use este sketch na ESP32-S3 para testar somente o HC-SR04.

## Ligações

- VCC do HC-SR04 → 5 V
- GND do HC-SR04 → GND da ESP32-S3
- TRIG → GPIO 17
- ECHO → GPIO 18 através de divisor de tensão ou conversor de nível para 3,3 V

Abra o Monitor Serial em **115200 baud**. A cada meio segundo aparecerá a duração do pulso e a distância.

## Interpretação

- `distancia=... cm`: o sensor respondeu.
- `SEM ECO`: confira primeiro alimentação, GND comum e a ligação TRIG/ECHO.
- `pulso` muito variável: use fios curtos, uma fonte estável e teste com um objeto plano a 10–100 cm.

Não conecte o ECHO de 5 V diretamente ao ESP32-S3.
