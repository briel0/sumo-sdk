#include "RCMode.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

void RCMode::init() {
    receptor.init();
}

void RCMode::handleWeapons(WeaponSystem &armas, int throttle, int steer) {
    if(receptor.Circle()) {
        _autoDisarmLocked = !_autoDisarmLocked;
        Serial.printf("[SUMÔ] Auto-desarme: %s\n", _autoDisarmLocked ? "TRAVADO" : "ATIVO");
    }

    if(!armas.isDeployed() && (throttle != 0 || steer != 0 || receptor.dpadUp())) {
        armas.deploy();
    }

    if(receptor.dpadDown() && armas.isDeployed() && !_autoDisarmLocked) {
        armas.retract();
    }

    if(!_autoDisarmLocked && armas.isDeployed() && throttle == 0 && steer == 0 && !macroPlayer.isPlaying()) {
        armas.retract();
    }
}

void RCMode::handleMacros(Drive &motores, WeaponSystem &armas) {
    if(macroPlayer.isPlaying()) {
        macroPlayer.update(motores);
        return;
    }

    auto triggerMacro = [&](const MotionSequence &seq) {
        if(!_autoDisarmLocked) {
            armas.deploy();
        }
        macroPlayer.play(seq);
    };

    if(receptor.Cross()) {
        triggerMacro(Config::MACRO_FRENTAO);
    }
    else if(receptor.Square()) {
        // triggerMacro(Config::MACRO_CURVAO_ESQ);
    }
    else if(receptor.Triangle()) {
        // triggerMacro(Config::MACRO_CURVAO_DIR);
    }
}

void RCMode::run(Drive &motores, WeaponSystem &armas) {
    receptor.update();
    armas.update();

    int throttle = receptor.rightTrigger() - receptor.leftTrigger();
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