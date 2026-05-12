#include <Arduino.h>
#include <Auto.hpp>
#include <RC.hpp>

#include <IRrecv.h>
#include <IRutils.h>

enum class RobotState {
    IDLE,
    RC,
    AUTO
};

enum class Mode {
    RC,
    AUTO,
    SELECTION
};

Mode currentMode = Mode::SELECTION;
RobotState currentState = RobotState::IDLE;

int IR_PIN = 13;
IRrecv irrecv(IR_PIN);
decode_results results;

void idle();

void setup() {
    Serial.begin(115200);
    Serial.println("Inicializando subsistemas.");
    irrecv.enableIRIn();
    currentState = RobotState::IDLE;
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
    delay(5);
}

RobotState lerIR() {
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
    if(currentMode == Mode::RC) {
        Serial.println("Modo RC forçado pelo código.");
        currentState = RobotState::RC;
        return;
    }
    else if(currentMode == Mode::AUTO) {
        Serial.println("Modo AUTO forçado pelo código.");
        currentState = RobotState::AUTO;
        return;
    }
    currentState = lerIR();
}