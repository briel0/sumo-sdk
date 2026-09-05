#include "AutoMode.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "IRreader.hpp"
#include "ServoMechanism.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>
#include <Wire.h>

// Firmware AUTO — desacoplado do RC (ver tools/RCFirmware/main.cpp) pra
// tirar o Bluepad32/BTstack do binário: eram duas stacks Bluetooth
// disputando o mesmo controller quando RC e AUTO viviam no mesmo main.cpp
// (suspeita levantada ao investigar o BLE do BleConfigServer que nunca
// aparecia em nenhum scanner, nem com todos os comandos HCI retornando OK).
// Trade-off aceito: perde a troca RC<->AUTO por IR em runtime, agora e'
// reflash pra trocar de modo.

Drive motores(Config::RIGHT_POS_PIN, Config::RIGHT_NEG_PIN, Config::LEFT_POS_PIN, Config::LEFT_NEG_PIN);
WeaponSystem sistemaDeArmas;
IRReader ir;
StatusLed statusLed;
AutoMode modoAuto;

ActiveAuto taticaAtual;

void setup() {
    Serial.begin(115200);
    Wire.begin(); // Inicializa I2C
    Serial.println("[MAIN] Inicializando subsistemas do Sumô (firmware AUTO).");
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

    modoAuto.init(taticaAtual);
    Serial.println("[MAIN] Modo AUTO engatilhado.");

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

    yield();
}
