#pragma once
#include "./RobotTypes.hpp"

// Perfil do mini trator: dois N20 de locomoção, um N20 no braço e dois
// servos. Pinos de locomoção/LED/servo 0 herdados do Caipora — conferir
// contra a placa do trator.
namespace Config {

    static constexpr const char *ROBOT_NAME = "Trator";

    // Locomoção (N20 x2)
    static constexpr int RIGHT_POS_PIN = 19;
    static constexpr int RIGHT_NEG_PIN = 18;
    static constexpr int LEFT_POS_PIN = 17;
    static constexpr int LEFT_NEG_PIN = 16;

    // Braço (N20) — setas cima/baixo, sempre em ARM_MAX_SPEED. Se subir
    // quando devia descer, troca ARM_INVERTED em vez de mexer na fiação.
    static constexpr int ARM_POS_PIN = 25;
    static constexpr int ARM_NEG_PIN = 32;
    static constexpr int ARM_MAX_SPEED = 60;
    static constexpr bool ARM_INVERTED = false;

    static constexpr int PIN_STATUS_LED = 33;
    static constexpr int STATUS_LED_COUNT = 5;

    // Servos: andam 1° a cada SERVO_MS_PER_DEG enquanto o botão estiver
    // segurado, presos entre os dois ângulos da linha; soltou, para e segura
    // a posição (o sinal nunca é cortado).
    // O servo liga no primeiro ângulo (retractAngle) — "voltar" anda na
    // direção dele, "ir" na direção do segundo (deployAngle).
    //   servo 0: L1 volta, L2 vai | servo 1: R1 volta, R2 vai
    static constexpr int NUM_SERVOS = 2;
    static constexpr int SERVO_MS_PER_DEG = 30; // ~33°/s — devagar de propósito

    // Faixa 20–160 em vez de 0–180: servo barato não alcança as pontas com
    // pulso de 500–2400 µs e fica forçando contra o batente interno. Se o
    // servo alcançar mais sem zumbir/esquentar, dá pra abrir aos poucos.
    static constexpr ServoConfig SERVOS[] = {
        {4, 20, 160},  // servo 0 | Pino 4
        {26, 20, 160}, // servo 1 | Pino 26
    };

}
