#include "AutoMode.hpp"
#include "CombatStrategy.hpp"
#include "Config.hpp"
#include "ConfigServer.hpp"
#include "HardwareCore.hpp"
#include "RobotTypes.hpp"
#include <Arduino.h>

static const MotionSequence *TABELA_DE_ESTRATEGIAS[] = {
    &Config::MACRO_FRENTAO,
};

void AutoMode::init(CombatStrategy &estrategia, HardwareCore &hardware) {
    Serial.println("Modo Auto Iniciado.");
    subState = SubState::SELECTING_ESTRATEGIA;
    autoConfig = AutoStrategy();
    configServer.begin();

    _estrategia = &estrategia;
    _hardware   = &hardware;

    // Inicializa junto com a estratégia (mesmo padrão de sempre): pinMode/analogRead
    // são chamadas síncronas e baratas, não competem com o AP que acabou de subir.
    if(Config::USES_HARDWARE_CORE) {
        _hardware->begin();
    }
    _estrategia->init(*_hardware);
}

// autoConfig é a struct com as configurações que vem do site :)

void AutoMode::run(Drive &motores, WeaponSystem &armas, bool irStart, bool irReady) {
    armas.update();

    // Varre sensores e aplica intenções de atuação uma vez por frame, no Core 1
    // (mesmo núcleo do combate). Ver HardwareCore.hpp para o porquê de não usar
    // uma task pinada no Core 0 aqui.
    if(_hardware && _hardware->isInitialized()) {
        _hardware->update();
    }

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
            }
            if(irStart) {
                _readyReceived = false;
                subState = SubState::EXECUTING_ESTRATEGIA;
                estrategiaPlayer.play(*TABELA_DE_ESTRATEGIAS[autoConfig.macro]);
                Serial.println("[AUTO] LARGADA! Executando estratégia.");
            }
            break;
        case SubState::EXECUTING_ESTRATEGIA:
            if(autoConfig.weapon) {
                armas.deploy();
            }
            executingEstrategia(motores);
            break;
        case SubState::FIGHTING:
            if(_estrategia && _hardware) {
                _estrategia->autoEngage(motores, armas, *_hardware);
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