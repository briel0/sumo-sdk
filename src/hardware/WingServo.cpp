#include "WingServo.hpp"

WingServo::WingServo(int pin, int retractAngle, int leftAngle, int rightAngle)
    : _pin(pin), _retractAngle(retractAngle), _leftAngle(leftAngle), _rightAngle(rightAngle) {}

void WingServo::init() {
    // Os timers LEDC são alocados uma vez em main.cpp setup(), antes de qualquer
    // servo prender o pino — não realocamos aqui pra não corromper a
    // contabilidade da ESP32PWM quando o robô também tem servos de WeaponSystem.
    _servo.setPeriodHertz(50);
    _servo.attach(_pin, 500, 2400);

    // Diagnóstico: se "attached" vier falso aqui, o problema é no attach() da
    // ESP32Servo (pino inválido/canal LEDC esgotado) — não em FumacinhaAuto
    // nem em HardwareCore, que só chamam setWing()/moveTo() depois disso.
    Serial.printf("[ASA] init() pino %d -> attached: %s\n", _pin, _servo.attached() ? "SIM" : "NAO (attach falhou!)");

    moveTo(WingPosition::RETRACTED);
}

void WingServo::moveTo(WingPosition position) {
    _position = position;

    int angle = _retractAngle;
    const char *label = "RETRACTED";
    switch(position) {
        case WingPosition::LEFT:
            angle = _leftAngle;
            label = "LEFT";
            break;
        case WingPosition::RIGHT:
            angle = _rightAngle;
            label = "RIGHT";
            break;
        case WingPosition::RETRACTED:
            angle = _retractAngle;
            label = "RETRACTED";
            break;
    }

    // Diagnóstico: confirma que ALGUÉM pediu esse movimento e para qual ângulo
    // calibrado ele foi traduzido — se isso aparecer no serial mas a asa física
    // não se mexer, o problema é elétrico (alimentação do servo, fio de sinal),
    // não de firmware.
    Serial.printf("[ASA] moveTo(%s) -> %d graus\n", label, angle);

    _writeAngle(angle);
}

void WingServo::_writeAngle(int angle) {
    if(!_servo.attached()) {
        Serial.println("[ASA] AVISO: servo estava desanexado, reanexando antes de escrever.");
        _servo.attach(_pin, 500, 2400);
    }
    angle = constrain(angle, 0, 180);
    _servo.write(angle);
}
