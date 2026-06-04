#pragma once
#include <stdint.h>

// Esta struct é apenas um envelope de dados. Não aciona hardware nenhum.
struct ServoConfig {
    int pin;
    int retractAngle;
    int deployAngle;
};

struct MotionStep {
    int leftSpeed;            // Velocidade do motor esquerdo (-100 a 100). Negativo = ré.
    int rightSpeed;           // Velocidade do motor direito  (-100 a 100). Negativo = ré.
    unsigned long durationMs; // Quanto tempo manter essa velocidade (em milissegundos).
};

// Representa uma fita cassete completa (uma lista de passos)
struct MotionSequence {
    const MotionStep *steps;
    int numSteps;
};

struct AutoStrategy {
    int macro = 0;        // Índice na TABELA_DE_ESTRATEGIAS[] do AutoMode.cpp
    char direction = 'X'; // 'E' = esquerda, 'D' = direita, 'X' = sem preferência
    int search = 1;       // 1 = busca padrão, 2 = busca lenta
    int weapon = 0;       // 0 = não armar, 1 = armar no começo da luta
    bool isNew = false;   // Sinaliza que chegou novos dados do celular
};

enum class Direction {
    left,
    right
};

// Isso serve pra deixar mais fácil na hora de escrever as macros no profile
#define MACRO(...)                                                                                                     \
    []() -> MotionSequence {                                                                                           \
        static constexpr MotionStep steps[] = {__VA_ARGS__};                                                           \
        return {steps, (int)(sizeof(steps) / sizeof(steps[0]))};                                                       \
    }()
