#include "AutoMode.hpp"
#include "CombatStrategy.hpp"
#include "Config.hpp"
#include "ConfigServer.hpp"
#include "RobotTypes.hpp"
#include <Arduino.h>

// Tabela de estratégias agora é carregada diretamente do Config::TABELA_MACROS_ESQ / DIR

// As duas tabelas sao indexadas pelo mesmo id vindo do site: precisam andar juntas.
static_assert(sizeof(Config::TABELA_MACROS_ESQ) == sizeof(Config::TABELA_MACROS_DIR),
              "TABELA_MACROS_ESQ e TABELA_MACROS_DIR precisam ter o mesmo numero de macros!");

static constexpr int NUM_MACROS = (int)(sizeof(Config::TABELA_MACROS_ESQ) / sizeof(Config::TABELA_MACROS_ESQ[0]));

static const MotionSequence MACRO_TESTE_MOTOR = MACRO(
    {60, 60, 500},
    {0, 0, 500},
    {-60, -60, 500},
    {0, 0, 500}
);

void AutoMode::init(CombatStrategy &estrategia) {
    Serial.println("Modo Auto Iniciado.");
    subState = SubState::SELECTING_ESTRATEGIA;
    autoConfig = AutoStrategy();

    configServer.setMacroTestCallback([this](MotionSequence seq) {
        _macroToTest = seq;
        _startMacroTest = true;
    });

    configServer.setMotorTestCallback([this](bool active) {
        if(!active)
            _testingMotor = false;
        else
            _testingMotor = true;
    });

    configServer.setSensorTestCallback([this](bool active) {
        if(!active)
            _testingSensor = false;
        else
            _testingSensor = true;
    });

    configServer.begin();

    _estrategia = &estrategia;
    _estrategia->init();
}

// autoConfig é a struct com as configurações que vem do site :)

void AutoMode::run(Drive &motores, WeaponSystem &armas, bool irStart, bool irReady) {
    armas.update();

    switch(subState) {
        case SubState::SELECTING_ESTRATEGIA:
            if(_testingSensor && _estrategia) {
                configServer.setTestReadout(_estrategia->getSensorStatusJSON());
            }
            configServer.update();

            if(_startMacroTest) {
                _startMacroTest = false;
                _testingMacro = true;
                _testingMotor = false;
                estrategiaPlayer.play(_macroToTest);
            }

            if(_testingMotor) {
                if(estrategiaPlayer.isPlaying()) {
                    estrategiaPlayer.update(motores);
                }
                else {
                    estrategiaPlayer.play(MACRO_TESTE_MOTOR);
                }
            }
            else if(_testingMacro) {
                if(estrategiaPlayer.isPlaying()) {
                    estrategiaPlayer.update(motores);
                }
                else {
                    motores.setSpeed(0, 0);
                    _testingMacro = false;
                }
            }
            else {
                if(estrategiaPlayer.isPlaying())
                    estrategiaPlayer.stop();
                motores.setSpeed(0, 0);
            }

            if(configServer.consumePayload(autoConfig)) {
                // Entrega o pacote tatico para a estrategia enquanto ainda da tempo:
                // daqui pra frente o radio morre e nao chega mais nada.
                if(_estrategia) {
                    _estrategia->configure(autoConfig);
                }
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

                // A arma abre (ou nao) junto com a estrategia inicial, nunca no
                // ready: ate a largada o robo tem que caber nas medidas.
                if(autoConfig.weapon) {
                    armas.deploy();
                }

                // O id vem cru de um parametro HTTP: sem isso, um indice invalido
                // vira ponteiro lixo desreferenciado no exato instante da largada.
                const int macroIdx = constrain(autoConfig.macro, 0, NUM_MACROS - 1);
                if(macroIdx != autoConfig.macro) {
                    Serial.printf("[AUTO] AVISO: macro %d fora da tabela (0..%d). Usando %d.\n", autoConfig.macro,
                                  NUM_MACROS - 1, macroIdx);
                }

                if(autoConfig.direction == 'D') {
                    estrategiaPlayer.play(*Config::TABELA_MACROS_DIR[macroIdx]);
                }
                else {
                    estrategiaPlayer.play(*Config::TABELA_MACROS_ESQ[macroIdx]);
                }
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