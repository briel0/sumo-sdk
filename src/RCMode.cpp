#include <Config.hpp>
#include <RCMode.hpp>
#include <WeaponSystem.hpp>

void RCMode::init() {
    receptor.init();
}

void RCMode::run(Drive &motores, WeaponSystem &armas) {
    receptor.update();

    int throttle = receptor.rightTrigger() - receptor.leftTrigger();
    int steer = receptor.leftStickX();

    if(receptor.btnCircle()) {
        _autoDisarmLocked = !_autoDisarmLocked;
        Serial.printf("[SUMÔ] Auto-desarme: %s\n", _autoDisarmLocked ? "TRAVADO" : "ATIVO");
    }

    if(!_isDeployed && (throttle != 0 || steer != 0)) {
        _isDeployed = true;
        Serial.println("[SUMÔ] Primeira movimentação: deploy.");
    }

    if(receptor.dpadUp() && !_isDeployed) {
        _isDeployed = true;
        Serial.println("[SUMÔ] Deploy manual.");
    }
    if(receptor.dpadDown() && _isDeployed && !_autoDisarmLocked) {
        _isDeployed = false;
        Serial.println("[SUMÔ] Desarme manual.");
    }

    if(_isDeployed) {
        if(!_isRelaxed) {
            if(_deployTime == 0) {
                armas.deployAll();
                _deployTime = millis();
            }
            if(millis() - _deployTime > 1000) {
                armas.relaxAll();
                _isRelaxed = true;
                Serial.println("[SUMÔ] Servos relaxados.");
            }
        }
    }
    else if(!_autoDisarmLocked) {
        armas.retractAll();
        _deployTime = 0;
        _isRelaxed = false;
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