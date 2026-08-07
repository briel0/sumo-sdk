#include "hardware/sensors/TSSP4038.hpp"

TSSP4038::TSSP4038(uint8_t pin) : _pin(pin) {}

void TSSP4038::init() const {
    // O chip TSSP4038 já possui resistor de pull-up interno no seu circuito,
    // portanto declarar apenas como INPUT é o método mais seguro.
    pinMode(_pin, INPUT);
}

bool TSSP4038::temOponente() const {
    // Abstração da lógica Active LOW: se a leitura for 0 (LOW), oponente detectado.
    return digitalRead(_pin) == LOW;
}

bool TSSP4038::leituraRaw() const {
    return digitalRead(_pin);
}