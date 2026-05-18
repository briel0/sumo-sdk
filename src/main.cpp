#include <Arduino.h>
#include <Config.hpp>
#include <Drive.hpp>
#include <RCMode.hpp>
#include <ServoMechanism.hpp>
#include <WeaponSystem.hpp>

#include <IRrecv.h>
#include <IRutils.h>

enum class RobotState {
    IDLE,
    RC,
    AUTO
};

RobotState currentState = RobotState::RC;

// Configurando o IR
constexpr int IR_PIN = 13;
IRrecv irrecv(IR_PIN);

// Configurando o objeto que controla os motores
Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);

RCMode modoRC;
WeaponSystem sistemaDeArmas;

void handleIdle();
RobotState lerIR();

static ServoMechanism servos[4];

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô.");

    irrecv.enableIRIn();

    for(int i = 0; i < Config::NUM_SERVOS; i++) {
        servos[i] =
            ServoMechanism(Config::SERVOS[i].pin, Config::SERVOS[i].retractAngle, Config::SERVOS[i].deployAngle);
        servos[i].init();
        sistemaDeArmas.addServo(&servos[i]);
    }
    modoRC.init();
}

void loop() {
    switch(currentState) {
        case RobotState::IDLE:
            handleIdle();
            break;
        case RobotState::RC:
            modoRC.run(motores, sistemaDeArmas);
            break;
        case RobotState::AUTO:
            break;
    }
    vTaskDelay(1);
}

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
    return currentState;
}

void handleIdle() {
    currentState = lerIR();
}