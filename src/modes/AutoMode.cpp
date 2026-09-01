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
    _benchTested = false;
}

// autoConfig é a struct com as configurações que vem do site :)

void AutoMode::run(Drive &motores, WeaponSystem &armas, bool irStart, bool irReady, StatusLed &statusLed) {
    armas.update();

    // Varre sensores e aplica intenções de atuação uma vez por frame, no Core 1
    // (mesmo núcleo do combate). Ver HardwareCore.hpp para o porquê de não usar
    // uma task pinada no Core 0 aqui.
    if(_hardware && _hardware->isInitialized()) {
        _hardware->update();
    }

    // FIGHTING assume o frame inteiro: nada de overlay de diagnóstico nem de
    // testador de macro durante a luta.
    if(subState == SubState::FIGHTING) {
        if(_estrategia && _hardware) {
            _estrategia->autoEngage(motores, armas, *_hardware);
        }
        return;
    }

    // Diagnóstico de bancada da HUD do Fumacinha (padrão polled: a rota só
    // guarda o pedido, quem aciona é aqui, fora da task async do HTTP).
    //
    // Atrás da flag de propósito: a HUD legada tem o SEU próprio caminho de
    // teste, por callback, dentro de SELECTING_ESTRATEGIA. Como a rota /set-test
    // aceita os nomes das duas HUDs, deixar os dois caminhos ativos no mesmo
    // robô poria dois MotionPlayer comandando os motores no mesmo frame.
    bool testeDeBancadaAtivo = false;

    if(Config::USES_FUMACINHA_FSM) {
        int macroCount = 0;
        if(configServer.consumeMacroTest(_macroSteps, 8, macroCount) && macroCount > 0) {
            configServer.clearTests();
            _macroPlayer.play({_macroSteps, macroCount});
        }
        if(_macroPlayer.isPlaying()) {
            _macroPlayer.update(motores);
            configServer.update();
            return;
        }

        if(configServer.isSensorTestActive()) {
            runSensorTestCycle(motores, statusLed);
            _benchTested = true;
            testeDeBancadaAtivo = true;
        }
        else if(configServer.isMotorTestActive()) {
            runMotorTestCycle(motores, statusLed);
            _benchTested = true;
            testeDeBancadaAtivo = true;
        }
        else if(configServer.isServoTestActive()) {
            runServoTestCycle(statusLed);
            _benchTested = true;
            testeDeBancadaAtivo = true;
        }
    }

    // Feedback de LED — o AutoMode é o dono único, ver o doc de run() no header.
    // Só roda quando nenhum teste de bancada está desenhando nos mesmos LEDs:
    // os dois escrevendo no mesmo frame era o que fazia o painel piscar.
    if(!testeDeBancadaAtivo) {
        if(subState == SubState::READY) {
            if(irReady) {
                statusLed.blinkDebug(1, 20);
                statusLed.setAll(CRGB::Red);
            }
            if(!_readyReceived) {
                statusLed.setState(CRGB::Orange);
            }
            if(irStart) {
                statusLed.setState(CRGB::Black);
            }
        }
        else if(_benchTested) {
            statusLed.setState(CRGB::Green);
        }
        else {
            statusLed.strategyWave();
        }
    }

    switch(subState) {
        case SubState::SELECTING_ESTRATEGIA:
            configServer.update();
            // Duas HUDs, dois payloads. Qual vale é decidido em tempo de
            // compilação pela MESMA flag que o ConfigServer usa pra escolher a
            // página servida — uma fonte de verdade só, sem chance de servir uma
            // HUD e esperar o payload da outra.
            if(Config::USES_FUMACINHA_FSM) {
                if(configServer.consumeCombatProfile(combatProfile)) {
                    // A estratégia recebe o pacote ANTES da largada. Quem não é
                    // a FumacinhaAuto herda o no-op da CombatStrategy.
                    if(_estrategia) {
                        _estrategia->setCombatProfile(combatProfile);
                    }
                    _tempoDesligamento = millis();
                    subState = SubState::DISCONNECTING_WIFI;
                    Serial.println("[AUTO] Perfil tático recebido. Aguardando buffer HTTP...");
                }
            }
            else if(configServer.consumePayload(autoConfig)) {
                _tempoDesligamento = millis();
                subState = SubState::DISCONNECTING_WIFI;
                Serial.println("[AUTO] Estratégia recebida. Dando tempo para o rádio responder...");
            }
            break;
        case SubState::DISCONNECTING_WIFI:
            if(millis() - _tempoDesligamento > 500) {
                configServer.shutdown();
                configServer.clearTests();
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

                // A abertura do Fumacinha é da própria estratégia (macro cega
                // escolhida na HUD, tocada pelo MotionPlayer dela), então não
                // existe saque cego separado aqui: vai direto pro combate.
                if(Config::USES_FUMACINHA_FSM) {
                    subState = SubState::FIGHTING;
                    Serial.println("[AUTO] LARGADA! Combate iniciado.");
                    break;
                }

                subState = SubState::EXECUTING_ESTRATEGIA;
                // A HUD legada oferece 8 botões de macro (0..7), mas a tabela só
                // tem o FRENTÃO — qualquer outro botão indexava fora do array.
                // Sem sensor de largada pra corrigir isso em pista, cai no 0.
                int macro = autoConfig.macro;
                if(macro < 0 || macro >= (int)(sizeof(TABELA_DE_ESTRATEGIAS) / sizeof(TABELA_DE_ESTRATEGIAS[0]))) {
                    Serial.printf("[AUTO] Macro %d não existe na tabela. Usando o FRENTÃO.\n", macro);
                    macro = 0;
                }
                estrategiaPlayer.play(*TABELA_DE_ESTRATEGIAS[macro]);
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
            break; // tratado no topo de run(), antes de qualquer overlay
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

void AutoMode::runSensorTestCycle(Drive &motores, StatusLed &statusLed) {
    if(!_hardware) {
        return;
    }

    // Trava de segurança: os motores NUNCA se movem durante o teste de sensores.
    motores.setSpeed(0, 0);

    // LEDs 2/3 mostram os IRs puros (irLeftRaw/irRightRaw), não os JSumos: o IR
    // puro fica sempre energizado, então é a leitura crua mais confiável pra
    // conferir na bancada (o JSumo depende do emissor furtivo estar ligado).
    DiagnosticsPanel panel(statusLed);
    panel.showSensorReadings(_hardware->opponentOnRamp(), _hardware->lineLeft(), _hardware->irLeftRaw(),
                             _hardware->irRightRaw(), _hardware->lineRight());

    // Log ao vivo pro celular: leitura exata (booleano já filtrado + valor cru
    // de ADC quando aplicável) de cada sensor, não só o resumo dos 5 LEDs.
    String json = "{";
    json += "\"rampaLdr\":";
    json += (_hardware->opponentOnRamp() ? "true" : "false");
    json += ",\"rampaLdrRaw\":" + String(_hardware->ldrFiltered());
    json += ",\"linhaEsq\":";
    json += (_hardware->lineLeft() ? "true" : "false");
    json += ",\"linhaEsqRaw\":" + String(_hardware->lineLeftRaw());
    json += ",\"linhaDir\":";
    json += (_hardware->lineRight() ? "true" : "false");
    json += ",\"linhaDirRaw\":" + String(_hardware->lineRightRaw());
    json += ",\"jsumoEsq\":";
    json += (_hardware->jsumoLeftRaw() ? "true" : "false");
    json += ",\"jsumoDir\":";
    json += (_hardware->jsumoRightRaw() ? "true" : "false");
    json += ",\"jsumoAsa\":";
    json += (_hardware->jsumoAsaRaw() ? "true" : "false");
    json += ",\"irEsq\":";
    json += (_hardware->irLeftRaw() ? "true" : "false");
    json += ",\"irDir\":";
    json += (_hardware->irRightRaw() ? "true" : "false");
    json += "}";

    configServer.setTestReadout(json);
}

void AutoMode::runMotorTestCycle(Drive &motores, StatusLed &statusLed) {
    if(millis() - _motorTestStartMs >= MOTOR_TEST_STEP_MS) {
        _motorTestStartMs = millis();
        switch(_motorTestState) {
            case MotorTestState::PARADO:
                _motorTestState = MotorTestState::FRENTE;
                break;
            case MotorTestState::FRENTE:
                _motorTestState = MotorTestState::TRAS;
                break;
            case MotorTestState::TRAS:
                _motorTestState = MotorTestState::ESQUERDA;
                break;
            case MotorTestState::ESQUERDA:
                _motorTestState = MotorTestState::DIREITA;
                break;
            case MotorTestState::DIREITA:
                _motorTestState = MotorTestState::PARADO;
                break;
        }
    }

    applyMotorTestState(motores);

    DiagnosticsPanel panel(statusLed);
    panel.showMotionVector(_motorTestState);

    // Log ao vivo pro celular: o PWM exato que acabou de ser ENVIADO pra cada
    // motor (não uma leitura de volta — Drive não guarda getter público, e não
    // precisa: quem chamou setSpeed() já sabe o valor).
    int leftPWM = 0, rightPWM = 0;
    motorPwmForState(_motorTestState, leftPWM, rightPWM);

    String json = "{\"estado\":\"";
    json += motorTestStateLabel(_motorTestState);
    json += "\",\"motorEsq\":" + String(leftPWM) + ",\"motorDir\":" + String(rightPWM) + "}";

    configServer.setTestReadout(json);
}

void AutoMode::applyMotorTestState(Drive &motores) {
    int leftPWM = 0, rightPWM = 0;
    motorPwmForState(_motorTestState, leftPWM, rightPWM);
    motores.setSpeed(leftPWM, rightPWM);
}

void AutoMode::motorPwmForState(MotorTestState state, int &leftPWM, int &rightPWM) const {
    switch(state) {
        case MotorTestState::PARADO:
            leftPWM = 0;
            rightPWM = 0;
            break;
        case MotorTestState::FRENTE:
            leftPWM = MOTOR_TEST_PWM;
            rightPWM = MOTOR_TEST_PWM;
            break;
        case MotorTestState::TRAS:
            leftPWM = -MOTOR_TEST_PWM;
            rightPWM = -MOTOR_TEST_PWM;
            break;
        case MotorTestState::ESQUERDA:
            leftPWM = -MOTOR_TEST_PWM;
            rightPWM = MOTOR_TEST_PWM;
            break;
        case MotorTestState::DIREITA:
            leftPWM = MOTOR_TEST_PWM;
            rightPWM = -MOTOR_TEST_PWM;
            break;
    }
}

const char *AutoMode::motorTestStateLabel(MotorTestState state) {
    switch(state) {
        case MotorTestState::PARADO:
            return "PARADO";
        case MotorTestState::FRENTE:
            return "FRENTE";
        case MotorTestState::TRAS:
            return "TRAS";
        case MotorTestState::ESQUERDA:
            return "ESQUERDA";
        case MotorTestState::DIREITA:
            return "DIREITA";
    }
    return "PARADO";
}

void AutoMode::runServoTestCycle(StatusLed &statusLed) {
    if(!_hardware) {
        return;
    }

    if(millis() - _servoTestStartMs >= SERVO_TEST_STEP_MS) {
        _servoTestStartMs = millis();
        _servoTestStep = (_servoTestStep + 1) % 4;
    }

    // RETRACTED -> LEFT -> RETRACTED -> RIGHT -> (repete). Passa sempre por
    // RETRACTED entre os dois lados pra deixar bem visível cada transição.
    WingPosition target;
    const char *label;
    switch(_servoTestStep) {
        case 1:
            target = WingPosition::LEFT;
            label = "LEFT";
            break;
        case 3:
            target = WingPosition::RIGHT;
            label = "RIGHT";
            break;
        default:
            target = WingPosition::RETRACTED;
            label = "RETRACTED";
            break;
    }

    // Mesmo caminho de produção que a FumacinhaAuto usa em combate
    // (HardwareCore::setWing(), aplicado em HardwareCore::update() logo no
    // topo de run()) — não um attach() isolado só pra esse teste, senão o
    // teste passar não prova que o caminho real de combate funciona.
    _hardware->setWing(target);

    // Representação nos LEDs: espelha pra onde a asa aponta.
    DiagnosticsPanel panel(statusLed);
    panel.showServoState(target);

    String json = "{\"estado\":\"";
    json += label;
    json += "\"}";
    configServer.setTestReadout(json);
}
