#include <Config.hpp>
#include <RCMode.hpp>
#include <WeaponSystem.hpp>

void RCMode::init() {
    receptor.init();
}

void RCMode::run(Drive &motores, WeaponSystem &armas) {
    receptor.update();
    armas.update();

    int throttle = receptor.rightTrigger() - receptor.leftTrigger();
    int steer = receptor.leftStickX();

    // Alterna o lock de desarme automático
    if(receptor.btnCircle()) {
        _autoDisarmLocked = !_autoDisarmLocked;
        Serial.printf("[SUMÔ] Auto-desarme: %s\n", _autoDisarmLocked ? "TRAVADO" : "ATIVO");
    }

    // Arma na primeira movimentação
    if(!armas.isDeployed() && (throttle != 0 || steer != 0))
        armas.deploy();

    // Arme/desarme manual pelo dpad
    if(receptor.dpadUp() && !armas.isDeployed())
        armas.deploy();

    if(receptor.dpadDown() && armas.isDeployed() && !_autoDisarmLocked)
        armas.retract();

    // Desarme automático ao soltar joystick
    if(!_autoDisarmLocked && armas.isDeployed() && throttle == 0 && steer == 0)
        armas.retract();

    throttle = (throttle * Config::MAX_THROTTLE) / 100;
    if(throttle == 0) {
        steer = (steer * Config::PIVOT_COEFFICIENT) / 100;
    }
    else {
        steer = (steer * Config::TURN_COEFFICIENT) / 100;
    }
    motores.setSpeed(throttle + steer, throttle - steer);
}