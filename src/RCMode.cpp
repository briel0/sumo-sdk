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

    // Arma na primeira movimentação ou pelo dpad
    if(!armas.isDeployed() && (throttle != 0 || steer != 0 || receptor.dpadUp())) {
        armas.deploy();
    }

    // Desarme manual pelo dpad
    if(receptor.dpadDown() && armas.isDeployed() && !_autoDisarmLocked) {
        armas.retract();
    }

    // Desarme automático ao soltar joystick
    if(!_autoDisarmLocked && armas.isDeployed() && throttle == 0 && steer == 0) {
        armas.retract();
    }
}

void RCMode::run(Drive &motores, WeaponSystem &armas) {
    receptor.update();
    armas.update();

    if(macroPlayer.isPlaying()) {
        macroPlayer.update(motores);
        return;
    }

    if(receptor.Cross()) {
        if(!armas.isDeployed()) {
            armas.deploy();
        }
        macroPlayer.play(Config::MACRO_FRENTAO);
        return;
    }

    int throttle = receptor.rightTrigger() - receptor.leftTrigger();
    int steer = receptor.leftStickX();

    handleWeapons(armas, throttle, steer);

    throttle = (throttle * Config::MAX_THROTTLE) / 100;
    if(throttle == 0) {
        steer = (steer * Config::PIVOT_COEFFICIENT) / 100;
    }
    else {
        steer = (steer * Config::TURN_COEFFICIENT) / 100;
    }

    motores.setSpeed(throttle + steer, throttle - steer);
}