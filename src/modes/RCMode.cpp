#include "RCMode.hpp"
#include "ArmMotor.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

void RCMode::init() {
    receptor.init();
}

void RCMode::handleArm(ArmMotor &braco) {
    int dir = 0;
    if(receptor.dpadUpHeld()) {
        dir = 1;
    }
    else if(receptor.dpadDownHeld()) {
        dir = -1;
    }
    if(Config::ARM_INVERTED) {
        dir = -dir;
    }
    braco.setSpeed(dir * Config::ARM_MAX_SPEED);
}

void RCMode::handleServos(WeaponSystem &armas) {
    // L2/R2 são analógicos (0..100); a partir daqui conta como apertado.
    static constexpr int TRIGGER_PRESSED = 30;

    // 1 = ir (rumo ao deployAngle), -1 = voltar, 0 = parado. Os dois
    // apertados ao mesmo tempo se anulam.
    int dir[2] = {
        (receptor.leftTrigger() > TRIGGER_PRESSED) - receptor.l1Held(),
        (receptor.rightTrigger() > TRIGGER_PRESSED) - receptor.r1Held(),
    };

    unsigned long now = millis();
    for(int i = 0; i < 2; i++) {
        if(dir[i] != 0 && now - _servoLastStep[i] >= Config::SERVO_MS_PER_DEG) {
            armas.nudgeServo(i, dir[i]);
            _servoLastStep[i] = now;
        }
    }
}

// Segurar Start por HOLD_MS cicla a polaridade dos motores (ver
// MotorPolarity.hpp) sem precisar do site. Start não é usado em mais nada em
// RCMode — mas um toque rápido (encostar sem querer) não é suficiente,
// precisa segurar de propósito. Ciclar só avança (0..7, dá a volta) — não
// faz sentido "voltar", o operador vai testando os motores até achar a
// config certa.
void RCMode::handleMotorPolarity(Drive &motores, StatusLed &led) {
    static constexpr unsigned long HOLD_MS = 700;
    // Purple: cor que StatusLed ainda não usa pra mais nada (Red/Orange/
    // Green já têm significado — boot, pareamento, conectado), e mais fácil
    // de distinguir delas a olho do que Blue. Flash sólido e bloqueante —
    // mesmo padrão de confirmStep()/blinkDebug(), aceitável aqui porque é um
    // gesto deliberado de bancada, não o hot path de pilotagem.
    static constexpr int FLASH_MS = 250;

    if(!receptor.startHeld()) {
        _polarityHoldStart = 0;
        _polarityArmed = true;
        return;
    }

    if(_polarityHoldStart == 0) {
        _polarityHoldStart = millis();
    }

    if(_polarityArmed && millis() - _polarityHoldStart >= HOLD_MS) {
        _polarityArmed = false;
        uint8_t idx = motores.cyclePolarity();
        Serial.printf("[RC] Start segurado %lums: polaridade -> config #%u\n", HOLD_MS, idx);
        led.setAll(CRGB::Purple);
        delay(FLASH_MS);
    }
}

void RCMode::run(Drive &motores, ArmMotor &braco, WeaponSystem &armas, StatusLed &led) {
    receptor.update();

    handleMotorPolarity(motores, led);

    // Um analógico por lado, sem limite: stick no fundo = 100%. Stick pra
    // cima vem negativo do Bluepad32 — inverte pra "cima = frente".
    int left = -receptor.leftStickY();
    int right = -receptor.rightStickY();

    handleArm(braco);

    handleServos(armas);

    motores.setSpeed(left, right);
}
