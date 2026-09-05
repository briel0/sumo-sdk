#include "Config.hpp"
#include "Drive.hpp"
#include "IRreader.hpp"
#include "RCMode.hpp"
#include "ServoMechanism.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

// Firmware RC — desacoplado do AUTO (ver src/main.cpp) pra tirar o
// Bluepad32/BTstack do binário do AUTO: eram duas stacks Bluetooth
// disputando o mesmo controller quando os dois modos viviam no mesmo
// main.cpp. Comportamento igual ao branch RC que existia lá antes da
// separação — só perdeu a troca por IR em runtime, agora é reflash pra
// trocar de modo.

Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
WeaponSystem sistemaDeArmas;
IRReader ir;
StatusLed statusLed;
RCMode modoRC;

void setup() {
    Serial.begin(115200);
    Serial.println("[MAIN] Inicializando subsistemas do Sumô (firmware RC).");
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
    modoRC.run(motores, sistemaDeArmas);

    yield();
}
