#include "MotionPlayer.hpp"
#include "Drive.hpp"
#include <Arduino.h>

void MotionPlayer::play(const MotionSequence &seq, int distancePercent, unsigned long startDelayMs) {
    const bool vazia = seq.numSteps <= 0 || seq.steps == nullptr;

    // Macro vazia = "sem saque". Sem espera inicial nem chega a tocar: o AutoMode
    // ve isPlaying() falso no primeiro frame e cai direto no combate.
    if(vazia && startDelayMs == 0) {
        sequence = {nullptr, 0};
        active = false;
        Serial.println("[MACRO] Vazia. Pulando saque cego.");
        return;
    }

    sequence = vazia ? MotionSequence{nullptr, 0} : seq;
    this->distancePercent = distancePercent;
    this->startDelayMs = startDelayMs;
    currentStep = 0;
    playStartTime = millis();
    stepStartTime = playStartTime;
    active = true;
    Serial.printf("[MACRO] Iniciada. %d passos, espera inicial de %lums.\n", sequence.numSteps, startDelayMs);
}

void MotionPlayer::update(Drive &motores) {
    if(!active)
        return;

    // Espera inicial: parado ate o tempo passar. So entao o primeiro passo
    // comeca a contar.
    if(startDelayMs > 0) {
        if(millis() - playStartTime < startDelayMs) {
            motores.setSpeed(0, 0);
            return;
        }
        startDelayMs = 0;
        stepStartTime = millis();

        if(sequence.numSteps <= 0) {
            active = false; // Macro vazia: era so a espera
            Serial.println("[MACRO] Espera inicial finalizada. Sem saque.");
            return;
        }
    }

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