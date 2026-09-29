#include "ArmMotor.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "IRreader.hpp"
#include "RCMode.hpp"
#include "ServoMechanism.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

// Firmware do mini trator: só RC (Bluepad32), sem sensor nem modo autônomo.
// Analógicos (vertical) = motor esquerdo/direito | setas cima/baixo = braço |
// L1/L2 = servo 0, R1/R2 = servo 1 (ver RCMode.cpp).

Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
ArmMotor braco(Config::ARM_POS_PIN, Config::ARM_NEG_PIN);
WeaponSystem sistemaDeArmas;
IRReader ir;
StatusLed statusLed;
RCMode modoRC;

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô (firmware do trator).");

    motores.initPolarity();
    Serial.printf("[MAIN] Polaridade dos motores: config #%u\n", motores.currentPolarity());
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

    modoRC.init();
    ir.shutdown();
    Serial.println("[MAIN] Modo RC engatilhado.");

    statusLed.confirmStep();
    statusLed.confirmStep();
    statusLed.confirmStep();
}

void loop() {
    ir.update();

    if(ir.stop()) {
        motores.setSpeed(0, 0);
        braco.setSpeed(0);
        Serial.println("[MAIN] COMANDO DE PARAGEM (3). Reiniciando o sistema...");
        delay(50);
        ESP.restart();
    }

    if(!modoRC.controllerConnected()) {
        statusLed.pairingWave();
    }
    else {
        statusLed.setState(CRGB::Green); // limpa o laranja residual
    }
    modoRC.run(motores, braco, sistemaDeArmas, statusLed);

    yield();
}
