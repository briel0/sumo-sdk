#pragma once

// Esta struct é apenas um envelope de dados. Não aciona hardware nenhum.
struct ServoConfig {
    int pin;
    int retractAngle;
    int deployAngle;
};

// Representa um único passo de movimento no tempo
struct MotionStep {
    int leftSpeed;
    int rightSpeed;
    unsigned long durationMs;
};

// Representa uma fita cassete completa (uma lista de passos)
struct MotionSequence {
    const MotionStep *steps;
    int numSteps;
};

// Valores padrão definidos conforme os comentários
struct AutoStrategy {
    int macro = 1;        // Frentão (Padrão)
    char direction = 'X'; // 'X' para NENHUM
    int search = 1;       // Busca Padrão
    int weapon = 0;       // Não desarmar
    bool isNew = false;   // Flag para a máquina de estados saber que tem pacote novo
};

#define MACRO(...)                                                                                                     \
    []() -> MotionSequence {                                                                                           \
        static constexpr MotionStep steps[] = {__VA_ARGS__};                                                           \
        return {steps, (int)(sizeof(steps) / sizeof(steps[0]))};                                                       \
    }()