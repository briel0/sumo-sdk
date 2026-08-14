#include "ServoMechanism.hpp"

ServoMechanism::ServoMechanism(int pin, int retractAngle, int deployAngle)
    : _pin(pin), _retractAngle(retractAngle), _deployAngle(deployAngle) {}

void ServoMechanism::init() {
    // Os timers LEDC são alocados uma vez em main.cpp setup(), antes de qualquer
    // servo prender o pino — não realocamos aqui pra não corromper a
    // contabilidade da ESP32PWM quando o robô também tem um WingServo.
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
    // Keep servo attached and holding position - don't detach
    // Detaching causes the servo to lose PWM signal and return to default position
    // The servo will hold its current angle without continuously updating
}