#include "hardware/sensors/JS40F.hpp"

JS40F::JS40F(uint8_t pin) : _pin(pin) {}

void JS40F::init() const {
    pinMode(_pin, INPUT);
}

bool JS40F::temAlvo() const {
    // Essa é uma continha com binários que vê se o pino está ativo ou não.
    uint32_t estadoPino = (_pin < 32) ? ((GPIO.in >> _pin) & 0x1) : ((GPIO.in1.val >> (_pin - 32)) & 0x1);
    return estadoPino;
}