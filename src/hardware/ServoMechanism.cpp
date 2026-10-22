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

void ServoMechanism::write(int angle) {
    if(!_servo.attached()) {
        _servo.attach(_pin, 500, 2400);
    }
    angle = constrain(angle, 0, 180);
    _servo.write(angle);
}

void ServoMechanism::setAngle(int angle) {
    write(angle);
    _relaxPending = true;
    _relaxTimerStart = millis();
}

void ServoMechanism::deploy() {
    setAngle(_deployAngle);
}

void ServoMechanism::retract() {
    write(_retractAngle);
    _relaxPending = false;
}

void ServoMechanism::update() {
    if(_relaxPending && (millis() - _relaxTimerStart > RELAX_TIMEOUT_MS)) {
        relax();
        _relaxPending = false;
    }
}

void ServoMechanism::relax() {
    if(_servo.attached()) {
        _servo.detach();
    }
}
