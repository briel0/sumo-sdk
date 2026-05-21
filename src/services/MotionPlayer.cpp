#include "MotionPlayer.hpp"
#include "Drive.hpp"
#include <Arduino.h>

void MotionPlayer::play(const MotionSequence &seq) {
    sequence = seq;
    currentStep = 0;
    stepStartTime = millis();
    active = true;
    Serial.printf("[MACRO] Iniciada. %d passos.\n", seq.numSteps);
}

void MotionPlayer::update(Drive &motores) {
    if(!active)
        return;

    unsigned long elapsed = millis() - stepStartTime;
    const MotionStep &step = sequence.steps[currentStep];

    if(elapsed < step.durationMs) {
        // Ainda estamos dentro do tempo deste passo
        motores.setSpeed(step.leftSpeed, step.rightSpeed);
    }
    else {
        // Acabou o tempo do passo atual, avança para o próximo!
        currentStep++;

        if(currentStep >= sequence.numSteps) {
            active = false; // A macro inteira acabou
            Serial.println("[MACRO] Finalizada. Controle restaurado.");
        }
        else {
            stepStartTime = millis();
        }
    }
}