#include <Arduino.h>
#include <Config.hpp>
#include <RCMode.hpp>

#include <IRrecv.h>
#include <IRutils.h>

enum class RobotState {
    IDLE,
    RC,
    AUTO
};

// É aqui que muda pra ir direto pra RC, auto, etc.
RobotState currentState = RobotState::RC;

// Configurando o IR
int IR_PIN = 13;
IRrecv irrecv(IR_PIN);

// Esse é o objeto que comanda os motores
Drive motores(RIGHT_POS_PIN, RIGHT_NEG_PIN, LEFT_POS_PIN, LEFT_NEG_PIN);

// Esse é o objeto que combina os comando do controle com os motores
RCMode modoRC;

void idle();

void setup() {
    Serial.begin(115200);
    Serial.println("Inicializando subsistemas.");
    irrecv.enableIRIn();
    modoRC.init();
}

void loop() {
    switch(currentState) {
        case RobotState::IDLE:
            idle();
            break;
        case RobotState::RC:
            modoRC.run(motores);
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