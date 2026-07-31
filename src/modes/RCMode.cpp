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

    if(receptor.r3()) {
        armas.setServoAngle(0, 20);
        Serial.println("[SUMÔ] R3: Override de ângulo no servo 0");
    }

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
    if(receptor.cross()) {
        triggerMacro(Config::MACRO_FRENTAO);
    }
    else if(receptor.square()) {
        // triggerMacro(Config::MACRO_CURVAO_ESQ);
    }
    else if(receptor.triangle()) {
        // triggerMacro(Config::MACRO_CURVAO_DIR);
    }
    else if(receptor.dpadRight()) {
        // triggerMacro(Config::MACRO_CURVAO_DIR);
    }
    else if(receptor.dpadLeft()) {
        // triggerMacro(Config::MACRO_CURVAO_DIR);
    }

    if(macroPlayer.isPlaying()) {
        macroPlayer.update(motores);
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