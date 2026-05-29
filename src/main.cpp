#include "AutoMode.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "IRreader.hpp"
#include "RCMode.hpp"
#include "ServoMechanism.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

enum class RobotState {
    IDLE,
    RC,
    AUTO
};

RobotState currentState = RobotState::IDLE;

Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
WeaponSystem sistemaDeArmas;
IRReader ir;
RCMode modoRC;
AutoMode modoAuto;

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô.");

    ir.init(IR_PIN);

    for(int i = 0; i < Config::NUM_SERVOS; i++) {
        ServoMechanism *s =
            new ServoMechanism(Config::SERVOS[i].pin, Config::SERVOS[i].retractAngle, Config::SERVOS[i].deployAngle);
        s->init();
        sistemaDeArmas.addServo(s);
    }

    switch(currentState) {
        case RobotState::RC:
            modoRC.init();
            Serial.println("[MAIN] BOOT DIRETO: Modo RC engatilhado.");
            break;
        case RobotState::AUTO:
            modoAuto.init();
            Serial.println("[MAIN] BOOT DIRETO: Modo AUTO engatilhado.");
            break;
        case RobotState::IDLE:
            Serial.println("[MAIN] Setup concluído. Aguardando sinal IR do juiz...");
            break;
    }
}

void loop() {
    ir.update();

    // Arbitragem de Estado Inicial (Só permite trocar de modo se o robô estiver parado)
    if(currentState == RobotState::IDLE) {
        if(ir.modeRC()) {
            modoRC.init();
            currentState = RobotState::RC;
            Serial.println("[MAIN] -> MODO RC ENGATILHADO");
        }
        else if(ir.modeAuto()) {
            modoAuto.init();
            currentState = RobotState::AUTO;
            Serial.println("[MAIN] -> MODO AUTO ENGATILHADO");
        }
    }

    switch(currentState) {
        case RobotState::RC:
            modoRC.run(motores, sistemaDeArmas);
            break;
        case RobotState::AUTO:
            modoAuto.run(motores, sistemaDeArmas, ir.start());
            break;
        case RobotState::IDLE:
            // Task: Codar um LED de heartbeat
            break;
    }
    yield();
}