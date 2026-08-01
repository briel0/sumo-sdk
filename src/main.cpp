#include "AutoMode.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "IRreader.hpp"
#include "RCMode.hpp"
#include "ServoMechanism.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

enum class RobotState {
    IDLE,
    RC,
    AUTO
};

// Aqui mudamos pra ir pra RC ou AUTO direto
RobotState currentState = RobotState::RC;

Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
WeaponSystem sistemaDeArmas;
IRReader ir;
StatusLed statusLed;
RCMode modoRC;
AutoMode modoAuto;

ActiveAuto taticaAtual;

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô.");
    statusLed.init(LED_BUILTIN, Config::PIN_STATUS_LED, Config::STATUS_LED_COUNT);
    delay(500);

    ir.init(IR_PIN);
    statusLed.confirmStep();

    for(int i = 0; i < Config::NUM_SERVOS; i++) {
        ServoMechanism *s =
            new ServoMechanism(Config::SERVOS[i].pin, Config::SERVOS[i].retractAngle, Config::SERVOS[i].deployAngle);
        s->init();
        sistemaDeArmas.addServo(s);
    }
    statusLed.confirmStep();

    switch(currentState) {
        case RobotState::RC:
            modoRC.init();
            ir.shutdown();
            Serial.println("[MAIN] BOOT DIRETO: Modo RC engatilhado.");
            break;
        case RobotState::AUTO:
            modoAuto.init(taticaAtual);
            Serial.println("[MAIN] BOOT DIRETO: Modo AUTO engatilhado.");
            break;
        case RobotState::IDLE:
            Serial.println("[MAIN] Setup concluído. Aguardando sinal IR do juiz...");
            break;
    }

    statusLed.confirmStep();
    statusLed.confirmStep();
    statusLed.confirmStep();
}

void loop() {
    ir.update();

    // Feedback visual para códigos IRs válidos (menos '1')
    if(ir.modeRC() || ir.modeAuto() || ir.stop()) {
        statusLed.blinkDebug(5, 20);
    }

    if(ir.stop()) {
        motores.setSpeed(0, 0);
        Serial.println("[MAIN] COMANDO DE PARAGEM (3). Reiniciando o sistema...");
        delay(50);
        ESP.restart();
    }

    if(currentState == RobotState::IDLE) {
        if(ir.modeRC()) {
            modoRC.init();
            ir.shutdown();
            currentState = RobotState::RC;
            statusLed.setState(CRGB::Green);
            Serial.println("[MAIN] -> MODO RC ENGATILHADO");
        }
        else if(ir.modeAuto()) {
            modoAuto.init(taticaAtual);
            currentState = RobotState::AUTO;
            statusLed.setState(CRGB::Orange);
            Serial.println("[MAIN] -> MODO AUTO ENGATILHADO");
        }
    }

    switch(currentState) {
        case RobotState::RC:
            if(!modoRC.controllerConnected()) {
                statusLed.pairingWave();
            }
            else {
                statusLed.setState(CRGB::Green); // limpa o laranja residual
            }
            modoRC.run(motores, sistemaDeArmas);
            break;
        case RobotState::AUTO:
            if(modoAuto.getSubState() == AutoMode::SubState::SELECTING_ESTRATEGIA ||
               modoAuto.getSubState() == AutoMode::SubState::DISCONNECTING_WIFI) {
                statusLed.strategyWave();
            }
            if(modoAuto.getSubState() == AutoMode::SubState::READY) {
                if(ir.ready()) {
                    statusLed.blinkDebug(1, 20);
                    statusLed.setAll(CRGB::Red);
                }
                if(!modoAuto.readyReceived()) {
                    statusLed.setState(CRGB::Orange);
                }
                if(ir.start()) {
                    statusLed.setState(CRGB::Black);
                }
            }
            modoAuto.run(motores, sistemaDeArmas, ir.start(), ir.ready());
            break;
        case RobotState::IDLE:
            statusLed.heartbeat();
            break;
    }
    yield();
}