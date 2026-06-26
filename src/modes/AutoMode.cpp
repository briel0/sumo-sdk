#include "AutoMode.hpp"
#include "CombatStrategy.hpp"
#include "Config.hpp"
#include "ConfigServer.hpp"
#include "RobotTypes.hpp"
#include <Arduino.h>

static const MotionSequence *TABELA_DE_ESTRATEGIAS[] = {
    &Config::MACRO_FRENTAO,
};

void AutoMode::init(CombatStrategy &estrategia) {
    Serial.println("Modo Auto Iniciado.");
    subState = SubState::SELECTING_ESTRATEGIA;
    autoConfig = AutoStrategy();
    configServer.begin();

    _estrategia = &estrategia;
    _estrategia->init();
}

// autoConfig é a struct com as configurações que vem do site :)

void AutoMode::run(Drive &motores, WeaponSystem &armas, bool irStart, bool irReady) {
    armas.update();

    switch(subState) {
        case SubState::SELECTING_ESTRATEGIA:
            configServer.update();
            if(configServer.consumePayload(autoConfig)) {
                _tempoDesligamento = millis();
                subState = SubState::DISCONNECTING_WIFI;
                Serial.println("[AUTO] Estratégia recebida. Dando tempo para o rádio responder...");
            }
            break;
        case SubState::DISCONNECTING_WIFI:
            if(millis() - _tempoDesligamento > 500) {
                configServer.shutdown();
                subState = SubState::READY;
                Serial.println("[AUTO] Rádio morto. Aguardando largada IR.");
            }
            break;
        case SubState::READY:
            if(irReady) {
                _readyReceived = true;
                if(autoConfig.weapon) {
                    armas.deploy();
                }
            }
            if(irStart) {
                _readyReceived = false;
                subState = SubState::EXECUTING_ESTRATEGIA;
                estrategiaPlayer.play(*TABELA_DE_ESTRATEGIAS[autoConfig.macro]);
                Serial.println("[AUTO] LARGADA! Executando estratégia.");
            }
            break;
        case SubState::EXECUTING_ESTRATEGIA:
            executingEstrategia(motores);
            break;
        case SubState::FIGHTING:
            if(_estrategia) {
                _estrategia->autoEngage(motores, armas);
            }
            break;
    }
}

void AutoMode::executingEstrategia(Drive &motores) {
    // Futuramente vamos colocar aqui pra interromper a macro caso o sensor de linha enxergue e etc.
    if(estrategiaPlayer.isPlaying()) {
        estrategiaPlayer.update(motores);
    }
    else {
        subState = SubState::FIGHTING;
        Serial.println("[AUTO] Saque cego finalizado. Iniciando caçada.");
    }
}

/*
ATAQUE DE ASA
void AutoMode::ataquePadrao(Drive &motores) {

    if(sensorEsq.temAlvo()) {
        _ultimoLado = Direction::left;
    }
    else if(sensorDir.temAlvo()) {
        _ultimoLado = Direction::right;
    }

    // Frontal limpo ou dois laterais simultâneos — força bruta
    if(sensorFrontal.temAlvo()) {
        motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_MAX);
        return;
    }

    // Diagonal esquerda ou só esquerda — arco fechando para esquerda
    if(sensorEsq.temAlvo() && !sensorDir.temAlvo()) {
        motores.setSpeed(VEL_ATAQUE_REDUZIDA, VEL_ATAQUE_MAX);
        return;
    }

    // Diagonal direita ou só direita — arco fechando para direita
    if(sensorDir.temAlvo() && !sensorEsq.temAlvo()) {
        motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_REDUZIDA);
        return;
    }

    // Perdeu contato
    subState = SubState::HUNTING;
}
*/