#include "WebRCMode.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

void WebRCMode::handleWeapons(WeaponSystem &armas, const WebJoystickState &joyState) {
    // Edge detection para o botão Circle (Trava de desarme automático)
    if(joyState.btnCircle && !_lastCircle) {
        _autoDisarmLocked = !_autoDisarmLocked;
        Serial.printf("[WEB-RC] Auto-desarme: %s\n", _autoDisarmLocked ? "TRAVADO" : "ATIVO");
    }
    _lastCircle = joyState.btnCircle;

    bool playerIsMoving = joyState.y != 0 || joyState.x != 0;
    bool weaponsArmed = armas.isDeployed();

    // Triângulo = Levantar rampa (Equivale ao DpadUp no controle físico)
    if(joyState.btnTriangle && !weaponsArmed) {
        armas.deploy();
    }

    // Cruz (X) = Descer rampa (Equivale ao DpadDown no controle físico)
    if(joyState.btnCross && weaponsArmed) {
        armas.retract();
    }

    // Quadrado = Override (Equivale ao R3)
    if(joyState.btnSquare) {
        armas.setServoAngle(0, 20);
    }

    // Lógica inteligente de defesa (Erguer a rampa sozinho ao acelerar)
    if(!_autoDisarmLocked) {
        if(!weaponsArmed && playerIsMoving) {
            armas.deploy();
        }
    }
}

void WebRCMode::run(Drive &motores, WeaponSystem &armas, const WebJoystickState &joyState) {
    // Processa os servos primeiro
    handleWeapons(armas, joyState);

    // No joystick.html desenhamos a física para Y positivo ser "para cima" e X positivo "direita".
    int throttle = joyState.y;
    int steer = joyState.x;

    // Mesmos multiplicadores aerodinâmicos/táticos definidos no Config do robô para o controle Bluetooth
    throttle = (throttle * Config::MAX_THROTTLE) / 100;
    
    if(throttle == 0) {
        steer = (steer * Config::PIVOT_COEFFICIENT) / 100;
    } else {
        steer = (steer * Config::TURN_COEFFICIENT) / 100;
    }

    // Tank mixing standard
    motores.setSpeed(throttle + steer, throttle - steer);
}
