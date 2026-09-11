#include "RCMode.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

void RCMode::init() {
    receptor.init();
}

void RCMode::handleWeapons(WeaponSystem &armas, int throttle, int steer) {
    if(receptor.circle()) {
        _autoDisarmLocked = !_autoDisarmLocked;
        Serial.printf("[SUMÔ] Auto-desarme: %s\n", _autoDisarmLocked ? "TRAVADO" : "ATIVO");
    }

    bool playerIsMoving = throttle != 0 || steer != 0;
    bool weaponsArmed = armas.isDeployed();
    bool macroRunning = macroPlayer.isPlaying();
    bool autoDisarmFree = !_autoDisarmLocked;

    if(receptor.dpadUp() && !weaponsArmed) {
        armas.deploy();
    }

    if(receptor.dpadDown() && weaponsArmed) {
        armas.retract();
    }

#ifndef ROBOT_CAIPORA_RC
    // Só o Caipora RC tem esse override — os outros robôs não têm esse uso
    // pro servo 0 no controle manual.
#else
    if(receptor.l3()) {
        armas.setServoAngle(0, 0);
        Serial.println("[SUMÔ] L3: joga a arma pro 0");
    }
#endif

    if(autoDisarmFree) {
        if(!weaponsArmed && (playerIsMoving || macroRunning)) {
            armas.deploy();
        }
    }
}

void RCMode::handleMacros(Drive &motores, WeaponSystem &armas) {
    auto triggerMacro = [&](const MotionSequence &seq) {
        if(!_autoDisarmLocked) {
            armas.deploy();
        }
        macroPlayer.play(seq);
    };

    // CONFIGURE AS MACROS AQUI!!!
    int numMacros = sizeof(Config::TABELA_MACROS_ESQ) / sizeof(Config::TABELA_MACROS_ESQ[0]);

    if(receptor.square()) {
        if(numMacros > 1) triggerMacro(*Config::TABELA_MACROS_ESQ[1]);
    }
    else if(receptor.triangle()) {
        if(numMacros > 1) triggerMacro(*Config::TABELA_MACROS_DIR[1]);
    }
#if defined(ROBOT_MAROLA) || defined(ROBOT_ARRUELA)
    // MACRO_CURVINHA_DIREITA/ESQUERDA só existem nesses dois profiles (ver
    // include/profiles/) — sem esse guard, o RC do Caipora/Smoker não compila.
    else if(receptor.r1()) {
        triggerMacro(Config::MACRO_CURVINHA_DIREITA);
    }
    else if(receptor.l1()) {
        triggerMacro(Config::MACRO_CURVINHA_ESQUERDA);
    }
#endif

    if(macroPlayer.isPlaying()) {
        macroPlayer.update(motores);
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

void RCMode::run(Drive &motores, WeaponSystem &armas, StatusLed &led) {
    receptor.update();
    armas.update();

    handleMotorPolarity(motores, led);

    int throttle = receptor.rightTrigger() - receptor.leftTrigger();

    // Acelerador digital: X força 100% pra frente se o gatilho não estiver acionado.
    if(receptor.crossHeld()) {
        throttle = 100;
    }

    int steer = receptor.leftStickX();

    handleWeapons(armas, throttle, steer);

    handleMacros(motores, armas);

    if(macroPlayer.isPlaying()) {
        return;
    }

    throttle = (throttle * Config::MAX_THROTTLE) / 100;
    if(throttle == 0) {
        steer = (steer * Config::PIVOT_COEFFICIENT) / 100;
    }
    else {
        steer = (steer * Config::TURN_COEFFICIENT) / 100;
    }

    motores.setSpeed(throttle + steer, throttle - steer);
}