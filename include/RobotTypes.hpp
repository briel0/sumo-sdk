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

#define MOTION_SEQ(steps_array) {steps_array, (int)(sizeof(steps_array) / sizeof(steps_array[0]))}