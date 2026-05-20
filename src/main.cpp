#include <Arduino.h>
#include <IRremote.hpp>

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

RobotState currentState = RobotState::IDLE;

constexpr int IR_PIN = 13;

Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
RCMode modoRC;
WeaponSystem sistemaDeArmas;

RobotState lerIR() {
    if(IrReceiver.decode()) {
        uint32_t codigo = IrReceiver.decodedIRData.decodedRawData;
        IrReceiver.resume();

        Serial.printf("[IR] Código: 0x%X\n", codigo);

        if(codigo == 0x86) {
            Serial.println("[IR] MODO AUTÔNOMO.");
            return RobotState::AUTO;
        }
        else if(codigo == 0x87) {
            Serial.println("[IR] MODO RC.");
            return RobotState::RC;
        }
    }
    return currentState;
}

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô.");

    IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);

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