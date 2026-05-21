#include "AutoMode.hpp"
#include "Config.hpp"
#include "ConfigServer.hpp"
#include <Arduino.h>
// #include "Sensors.hpp"

void AutoMode::init() {
    Serial.println("Modo Auto Iniciado.");
    subState = SubState::SELECTING_SAQUE;
    autoConfig = AutoStrategy();
    configServer.begin();
}

void AutoMode::run(Drive &motores, WeaponSystem &armas, bool irArmed, bool irStart) {
    switch(subState) {
        case SubState::SELECTING_SAQUE:
            configServer.update();

            if(configServer.consumePayload(autoConfig)) {
                // delay(200);
                // Se o servidor der ruim descomente a linha acima.
                subState = SubState::ARMED_READY;
                configServer.shutdown();
            }
            break;

        case SubState::ARMED_READY:
            handleArmedReady(irStart);
            break;

        case SubState::EXECUTING_SAQUE:
            handleExecutingSaque(motores, armas);
            break;

        case SubState::HUNTING:
        case SubState::ATTACKING:
            // handleCombat(motores, armas, sensores);
            break;
    }
}

void AutoMode::handleArmedReady(bool irStart) {
    // Robô completamente cego para redes, esperando apenas a luz do IR.
    if(irStart) {
        subState = SubState::EXECUTING_SAQUE;
        Serial.println("[AUTO] Transição -> EXECUTING_SAQUE. ATAQUE INICIADO!");

        // Engatilha a fita cassete no MotionPlayer correspondente ao que foi salvo
        /*
        switch(saque) {
            case Saque::FRENTAO:    player.play(Config::MACRO_FRENTAO); break;
            case Saque::CURVAO_ESQ: player.play(Config::MACRO_CURVAO_ESQ); break;
            case Saque::CURVAO_DIR: player.play(Config::MACRO_CURVAO_DIR); break;
            case Saque::RECUADO:    player.play(Config::MACRO_RECUADO); break;
        }
        */
    }
}

void AutoMode::handleExecutingSaque(Drive &motores, WeaponSystem &armas) {
    // Se o sensor JS40F gritar ou a linha brilhar no meio do saque, nós o abortamos.
    // O combate de verdade sempre ignora a coreografia cega.
    /*
    if (sensores.viuLinha() || sensores.viuInimigo()) {
        player.stop(); // Interrompe a fita
        subState = SubState::HUNTING; // Joga pra máquina de combate resolver
        return;
    }
    */

    if(player.isPlaying()) {
        player.update(motores);
    }
    else {
        // O array de movimentos acabou e o inimigo não cruzou a frente.
        subState = SubState::HUNTING;
        Serial.println("[AUTO] Saque cego finalizado. Iniciando caçada.");
    }
}

void AutoMode::handleCombat(Drive &motores, WeaponSystem &armas) {
    // A Hierarquia de Interrupção Absoluta (A FSM do Combate)
    /*
    // 1. Prioridade Máxima: Sobrevivência
    if (sensores.viuLinha()) {
        subState = SubState::HUNTING;
        motores.setSpeed(-100, -100); // Ré total imediata
        return;
    }

    // 2. Prioridade Secundária: Oportunidade de Ataque
    if (sensores.viuInimigo()) {
        subState = SubState::ATTACKING;
        if (!armas.isDeployed()) armas.deploy();
        motores.setSpeed(100, 100); // Força bruta pra frente
    }
    // 3. Estado Base: Caça (Giro no eixo)
    else {
        subState = SubState::HUNTING;
        motores.setSpeed(50, -50); // Pivotando para encontrar o alvo
    }
    */
}