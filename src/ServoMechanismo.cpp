#include "ServoMechanism.hpp"

ServoMechanism::ServoMechanism(int pin, int retractAngle, int deployAngle)
    : _pin(pin), _retractAngle(retractAngle), _deployAngle(deployAngle) {}

void ServoMechanism::init() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    _servo.setPeriodHertz(50);

    _servo.attach(_pin, 500, 2400);

    retract();
}

void ServoMechanism::setAngle(int angle) {
    if(!_servo.attached()) {
        _servo.attach(_pin, 500, 2400);
    }
    angle = constrain(angle, 0, 180);
    _servo.write(angle);
}

void ServoMechanism::deploy() {
    setAngle(_deployAngle);
}

void ServoMechanism::retract() {
    setAngle(_retractAngle);
}

void ServoMechanism::relax() {
    if(_servo.attached()) {
        _servo.detach();
    }
}