#include "MotionPlayer.hpp"
#include "Drive.hpp"
#include <Arduino.h>

void MotionPlayer::play(const MotionSequence &seq, int distancePercent) {
    // Macro vazia = "sem saque". Nem chega a tocar: o AutoMode ve isPlaying() falso
    // no primeiro frame e cai direto no combate.
    if(seq.numSteps <= 0 || seq.steps == nullptr) {
        sequence = {nullptr, 0};
        active = false;
        Serial.println("[MACRO] Vazia. Pulando saque cego.");
        return;
    }

    sequence = seq;
    this->distancePercent = distancePercent;
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

    unsigned long duration = step.durationMs;
    if(step.scalesWithDistance) {
        duration = duration * distancePercent / 100;
    }

    if(elapsed < duration) {
        // Ainda estamos dentro do tempo deste passo
        motores.setSpeed(step.leftSpeed, step.rightSpeed);
    }
    else {
        // Acabou o tempo do passo atual, avança para o próximo!
        currentStep++;

        if(currentStep >= sequence.numSteps) {
            active = false; // A macro inteira acabou
            motores.setSpeed(0, 0); // Zera o PWM para garantir estado limpo ao devolver o controle
            Serial.println("[MACRO] Finalizada. Controle restaurado.");
        }
        else {
            stepStartTime = millis();
        }
    }
}