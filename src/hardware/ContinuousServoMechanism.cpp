#include "ContinuousServoMechanism.hpp"

ContinuousServoMechanism::ContinuousServoMechanism(int pin, int retractCmd, int deployCmd, unsigned long retractMs,
                                                   unsigned long deployMs)
    : _pin(pin), _retractCmd(retractCmd), _deployCmd(deployCmd), _retractMs(retractMs), _deployMs(deployMs) {}

void ContinuousServoMechanism::init() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    _servo.setPeriodHertz(50);

    // Recolhe no boot. Bloqueia de proposito: quem corta o giro e o update(),
    // que so comeca a rodar no loop() — sem o delay o servo ficaria girando
    // pelo resto do setup() inteiro, bem alem do _retractMs.
    spin(_retractCmd, _retractMs);
    delay(_retractMs);
    stop();
}

void ContinuousServoMechanism::deploy() {
    spin(_deployCmd, _deployMs);
}

void ContinuousServoMechanism::retract() {
    spin(_retractCmd, _retractMs);
}

void ContinuousServoMechanism::update() {
    if(_spinning && (millis() - _spinStart >= _spinDurationMs)) {
        stop();
    }
}

void ContinuousServoMechanism::spin(int cmd, unsigned long durationMs) {
    if(!_servo.attached()) {
        _servo.attach(_pin, 500, 2400);
    }
    _servo.write(constrain(cmd, 0, 180));
    _spinning = true;
    _spinStart = millis();
    _spinDurationMs = durationMs;
}

// Parar é cortar o sinal, não escrever 90: com o attach em 500–2400 µs o 90
// sai em 1450 µs, e o neutro do servo varia de peça pra peça — escrever 90
// deixaria ele rastejando. Sem pulso ele para de vez.
void ContinuousServoMechanism::stop() {
    if(_servo.attached()) {
        _servo.detach();
    }
    // Linha em LOW pra não flutuar e o servo não ler ruído como comando.
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    _spinning = false;
}
