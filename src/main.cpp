#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>

#include "Config.hpp"
#include "Drive.hpp"
#include "RCMode.hpp"
#include "ServoMechanism.hpp"
#include "WeaponSystem.hpp"

enum class RobotState {
    IDLE,
    RC,
    AUTO
};

// É que onde definimos se vai começar como auto, RC, ou modo seleção por IR.
RobotState currentState = RobotState::RC;

// Configuração do IR.
constexpr int IR_PIN = 13;
IRrecv irrecv(IR_PIN);

// Esse é o objeto que administra os motores.
Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);

// Esse é o objeto que administra o modo RC.
RCMode modoRC;

// Esse é o objeto que administra todos os servos do robô.
WeaponSystem sistemaDeArmas;

RobotState lerIR() {
    decode_results results;
    if(irrecv.decode(&results)) {
        uint32_t codigo = results.value;
        irrecv.resume();

        if(codigo == 0x7) {
            Serial.println("[IR] Comando recebido: MODO AUTÔNOMO.");
            return RobotState::AUTO;
        }
        else if(codigo == 0x8) {
            Serial.println("[IR] Comando recebido: MODO RC (RÁDIO).");
            return RobotState::RC;
        }
    }
    // Mantém o estado atual se for uma opção inválida
    return currentState;
}

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô.");

    irrecv.enableIRIn();

    // Colocando todos os servos no sistema de armas.
    for(int i = 0; i < Config::NUM_SERVOS; i++) {
        ServoMechanism *s =
            new ServoMechanism(Config::SERVOS[i].pin, Config::SERVOS[i].retractAngle, Config::SERVOS[i].deployAngle);
        s->init();
        sistemaDeArmas.addServo(s);
    }

    modoRC.init();
    Serial.println("[MAIN] Setup concluído. Aguardando comando IR...");
}

void loop() {
    switch(currentState) {
        case RobotState::IDLE:
            currentState = lerIR();
            break;
        case RobotState::RC:
            modoRC.run(motores, sistemaDeArmas);
            break;
        case RobotState::AUTO:
            break;
    }
    vTaskDelay(1);
}