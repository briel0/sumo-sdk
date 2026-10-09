#include "WeaponSystem.hpp"
#include "ContinuousServoMechanism.hpp"
#include "RobotTypes.hpp"
#include "ServoMechanism.hpp"
#include <Arduino.h>

void WeaponSystem::init() {
    for(int i = 0; i < servoCount; i++) {
        if(servos[i] != nullptr) {
            servos[i]->init();
        }
    }
}

void WeaponSystem::addServo(Mechanism *servo) {
    if(servoCount < MAX && servo != nullptr) {
        servos[servoCount] = servo;
        servoCount++;
    }
}

void WeaponSystem::addServo(const ServoConfig &config) {
    if(config.kind == ServoKind::Continuous) {
        addServo(new ContinuousServoMechanism(config.pin, config.retractAngle, config.deployAngle, config.retractMs,
                                              config.deployMs));
    }
    else {
        addServo(new ServoMechanism(config.pin, config.retractAngle, config.deployAngle));
    }
}

void WeaponSystem::deploy() {
    if(isDeployedFlag) {
        return;
    }
    deployAll();
    isDeployedFlag = true;
}

void WeaponSystem::retract() {
    if(!isDeployedFlag) {
        return;
    }
    retractAll();
    isDeployedFlag = false;
}

void WeaponSystem::update() {
    for(int i = 0; i < servoCount; i++) {
        if(servos[i] != nullptr) {
            servos[i]->update();
        }
    }
}

void WeaponSystem::deployAll() {
    for(int i = 0; i < servoCount; i++) {
        if(servos[i] != nullptr) {
            servos[i]->deploy();
        }
    }
}

void WeaponSystem::retractAll() {
    for(int i = 0; i < servoCount; i++) {
        if(servos[i] != nullptr) {
            servos[i]->retract();
        }
    }
}

void WeaponSystem::setServoAngle(int index, int angle) {
    if(index >= 0 && index < servoCount && servos[index] != nullptr) {
        servos[index]->setAngle(angle);
        // Sem isso, isDeployedFlag ficava do jeito que estava antes desse
        // comando (geralmente false) — e o auto-deploy do RCMode
        // (!weaponsArmed && robô se movendo) rodava logo em seguida e
        // sobrescrevia o ângulo pro _deployAngle padrão no mesmo frame (ou
        // no próximo). Marcar como "deployed" aqui é o que faz o ângulo
        // customizado realmente pegar, independente do estado anterior.
        isDeployedFlag = true;
    }
}
