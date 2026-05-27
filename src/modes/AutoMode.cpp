#include "AutoMode.hpp"
#include "Config.hpp"
#include "ConfigServer.hpp"
#include "RobotTypes.hpp"
#include <Arduino.h>

static const MotionSequence *TABELA_DE_ESTRATEGIAS[] = {
    &Config::MACRO_FRENTAO, // Index 0
};

AutoMode::AutoMode()
    : sensorEsq(Config::PIN_JS_ESQ), sensorDir(Config::PIN_JS_DIR), sensorFrontal(Config::PIN_JS_FRONT) {}

void AutoMode::init() {
    Serial.println("Modo Auto Iniciado.");
    subState = SubState::SELECTING_ESTRATEGIA;
    autoConfig = AutoStrategy();
    configServer.begin();
}

// autoConfig é a struct com as configurações que vem do site :)

// Esqueci de codar a parada :)

void AutoMode::run(Drive &motores, WeaponSystem &armas, bool irStart) {
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
            if(irStart) {
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

        case SubState::HUNTING:
            buscaPadrao(motores);
            break;
        case SubState::ATTACKING:
            ataquePadrao(motores);
            break;
    }
}

void AutoMode::executingEstrategia(Drive &motores) {
    // Futuramente vamos colocar aqui pra interromper a macro caso o sensor de linha enxergue e etc.
    if(estrategiaPlayer.isPlaying()) {
        estrategiaPlayer.update(motores);
    }
    else {
        subState = SubState::HUNTING;
        Serial.println("[AUTO] Saque cego finalizado. Iniciando caçada.");
    }
}

void AutoMode::buscaPadrao(Drive &motores) {
    // 1. Alvo cravado na frente: Fim da busca, transição imediata para ataque.
    if(sensorFrontal.temAlvo()) {
        subState = SubState::ATTACKING;
        return;
    }

    // 2. Reflexo Esquerdo: Salva na memória e corrige o eixo para centralizar
    if(sensorEsq.temAlvo()) {
        _ultimoLado = Direction::left;
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
        return;
    }

    // 3. Reflexo Direito: Salva na memória e corrige o eixo para centralizar
    if(sensorDir.temAlvo()) {
        _ultimoLado = Direction::right;
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
        return;
    }

    // 4. Cegueira Total (Varredura Tática): Gira confiando no último contato visual
    if(_ultimoLado == Direction::right) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    }
    else {
        // Padrão seguro: se nunca viu ninguém (ou viu na esquerda), varre pra esquerda
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
    }
}

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