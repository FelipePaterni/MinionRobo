#pragma once

#include <Arduino.h>

/**
 * Parâmetros ajustáveis do firmware.
 *
 * Os valores abaixo são pontos de partida. O giro de 180 graus, em especial,
 * depende do peso do robô, do piso, da bateria e da diferença entre motores.
 */
namespace Config {
  constexpr uint32_t PWM_FREQUENCY = 1000; // Hz do PWM.
  constexpr uint8_t PWM_RESOLUTION = 8; // Resolucao do PWM (0 a 255).
  constexpr int PWM_MAX = 255; // PWM maximo permitido.
  constexpr int PWM_START_MINIMUM = 130; // PWM minimo para o motor arrancar.

  constexpr int PWM_SEARCH = 120; // Velocidade durante a busca.
  constexpr int PWM_FOLLOW = 175; // Velocidade seguindo quando esta longe.
  constexpr int PWM_FOLLOW_SLOW = 140; // Velocidade dentro da faixa ideal.
  constexpr int PWM_ESCAPE = 210; // Velocidade para escapar.
  constexpr int PWM_SCAN = 120; // Velocidade das varreduras.
  constexpr int PWM_RECOVERY_TURN = 155; // Velocidade do giro de recuperacao.

  constexpr float DISTANCE_EMERGENCY = 15.0f; // cm; para por risco de colisao.
  constexpr float DISTANCE_MINIMUM = 28.0f; // cm; abaixo disso nao avanca.
  // Começa a seguir antes, evitando que o alvo precise chegar muito perto.
  constexpr float DISTANCE_DESIRED = 80.0f; // cm; distancia ideal ate o alvo.
  constexpr float DISTANCE_TOLERANCE = 50.0f; // cm; margem acima/abaixo da distancia ideal.
  // O HC-SR04 mede até aproximadamente 400 cm; uma margem de 3,5 m
  // permite detectar o alvo mais cedo sem tratar ecos muito distantes como alvo.
  constexpr float MAX_TARGET_DISTANCE = 350.0f; // cm; distancia maxima para reconhecer alvo.
  constexpr float TARGET_STATIONARY_TOLERANCE = 4.0f; // cm; variacao menor indica alvo parado.

  constexpr uint32_t SENSOR_READ_INTERVAL = 100; // ms entre leituras.
  constexpr uint32_t DIAGNOSTIC_INTERVAL = 500; // ms entre diagnosticos.
  constexpr uint32_t INITIALIZATION_TIME = 1500; // ms parado ao iniciar.
  constexpr uint32_t LOST_TARGET_TIME = 800; // ms sem alvo antes de buscar.
  constexpr uint32_t ESCAPE_MAX_TIME = 1800; // ms maximo tentando escapar.
  // Aguarda mais tempo antes de concluir que está encarando uma parede.
  constexpr uint32_t TARGET_STATIONARY_TIME = 10000; // ms ate considerar parede parada.
  constexpr uint32_t SEARCH_SEGMENT_MIN = 1200; // ms; menor trecho de busca.
  constexpr uint32_t SEARCH_SEGMENT_MAX = 2600; // ms; maior trecho de busca.
  constexpr uint32_t SEARCH_SCAN_INTERVAL = 5000; // ms entre varreduras na busca.
  constexpr uint32_t FOLLOW_SCAN_INTERVAL = 3000; // ms entre varreduras seguindo.
  constexpr uint32_t SCAN_STEP_TIME = 350; // ms de cada passo lateral.
  constexpr uint32_t SCAN_CORRECTION_TIME = 260; // ms corrigindo direcao.
  constexpr uint32_t RECOVERY_STOP_TIME = 350; // ms parado antes do giro.
  constexpr uint32_t TURN_180_TIME = 1150; // ms; calibre para 180 graus.
  constexpr uint32_t SAFE_STOP_HOLD_TIME = 700; // ms aguardando antes de retomar.
  constexpr uint32_t SAFE_STOP_MAX_TIME = 1800; // ms ate forcar giro.

  // true: LOW indica obstáculo; false: HIGH indica obstáculo.
  constexpr bool IR_ACTIVE_LOW = true; // true se obstaculo e sinalizado com LOW.

  constexpr bool INVERT_LEFT_MOTOR = false; // true para inverter motor esquerdo.
  constexpr bool INVERT_RIGHT_MOTOR = false; // true para inverter motor direito.

  // Ausência de eco é interpretada como área sem alvo, não como obstáculo.
  constexpr bool STOP_ON_INVALID_ULTRASONIC = false; // true para parar sem eco valido.

  constexpr bool ENABLE_DIAGNOSTICS = true; // true para manter diagnosticos ativos.
}
