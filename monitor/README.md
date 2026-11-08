# Monitor local do MinionRobo

1. Grave o firmware da pasta `main/` no ESP32-S3.
2. Conecte o computador à rede Wi-Fi `MinionRobo` usando a senha `minion123`.
3. Abra `index.html` neste diretório no navegador.
4. Mantenha o endereço `http://192.168.4.1` no campo **ESP**.

A página consulta somente `GET /api/status` e `GET /api/health`. Ela não envia
comandos de movimento; o firmware autônomo continua sendo o único responsável
por controlar os motores.

Se o endereço do ponto de acesso mudar, use o IP exibido no Monitor Serial.
