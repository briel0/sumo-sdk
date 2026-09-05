#include "RCMode.hpp"
#include "Config.hpp"
#include "Drive.hpp"
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
    else if(receptor.dpadRight()) {
        //triggerMacro(Config::MACRO_CURVINHA_DIREITA);
    }
    else if(receptor.dpadLeft()) {
        //triggerMacro(Config::MACRO_CURVINHA_ESQUERDA);
    }

    if(macroPlayer.isPlaying()) {
        macroPlayer.update(motores);
    }
}

void RCMode::run(Drive &motores, WeaponSystem &armas) {
    receptor.update();
    armas.update();

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