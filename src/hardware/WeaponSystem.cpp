#include "WeaponSystem.hpp"
#include "ServoMechanism.hpp"
#include <Arduino.h>

void WeaponSystem::init() {
    for(int i = 0; i < servoCount; i++) {
        if(servos[i] != nullptr) {
            servos[i]->init();
        }
    }
}

void WeaponSystem::addServo(ServoMechanism *servo) {
    if(servoCount < MAX && servo != nullptr) {
        servos[servoCount] = servo;
        servoCount++;
    }
}

void WeaponSystem::deploy() {
    if(isDeployedFlag) {
        return;
    }
    deployAll();
    isDeployedFlag = true;
    isRelaxedFlag = false;
    deployTimeStart = millis();
}

void WeaponSystem::retract() {
    if(!isDeployedFlag) {
        return;
    }
    retractAll();
    isDeployedFlag = false;
    isRelaxedFlag = false;
    deployTimeStart = 0;
}

void WeaponSystem::update() {
    if(isDeployedFlag && !isRelaxedFlag && (millis() - deployTimeStart > RELAX_TIMEOUT_MS)) {
        relaxAll();
        isRelaxedFlag = true;
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

void WeaponSystem::relaxAll() {
    for(int i = 0; i < servoCount; i++) {
        if(servos[i] != nullptr) {
            servos[i]->relax();
        }
    }
}

void WeaponSystem::setServoAngle(int index, int angle) {
    if(index >= 0 && index < servoCount && servos[index] != nullptr) {
        servos[index]->setAngle(angle);
        isRelaxedFlag = false;
    }
}