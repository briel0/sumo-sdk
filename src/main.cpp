#include <Arduino.h>
#include <RC.hpp>

#include <IRrecv.h>
#include <IRutils.h>

enum class RobotState {
    IDLE,
    RC,
    AUTO
};

// É aqui que muda pra ir direto pra RC, auto, etc.
RobotState currentState = RobotState::IDLE;

int IR_PIN = 13;
IRrecv irrecv(IR_PIN);

void idle();

void setup() {
    Serial.begin(115200);
    Serial.println("Inicializando subsistemas.");
    irrecv.enableIRIn();
}

void loop() {
    switch(currentState) {
        case RobotState::IDLE:
            idle();
            break;
        case RobotState::RC:
            break;
        case RobotState::AUTO:
            break;
    }
}

RobotState lerIR() {
    decode_results results;
    if(irrecv.decode(&results)) {
        uint32_t codigo = results.value;
        irrecv.resume();

        if(codigo == 0x7) {
            Serial.println("Modo AUTÔNOMO.");
            return RobotState::AUTO;
        }
        else if(codigo == 0x8) {
            Serial.println("Modo RC.");
            return RobotState::RC;
        }
    }
    return RobotState::IDLE;
}

void idle() {
    currentState = lerIR();
}