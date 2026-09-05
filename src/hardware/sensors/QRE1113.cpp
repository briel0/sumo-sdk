#include "hardware/sensors/QRE1113.hpp"

QRE1113::QRE1113(uint8_t pin, uint16_t threshold) : _pin(pin), _threshold(threshold) {}

void QRE1113::init() const {
    pinMode(_pin, INPUT);
}

bool QRE1113::temLinhaBranca() const {
    return analogRead(_pin) > _threshold;
}

uint16_t QRE1113::leituraRaw() const {
    return analogRead(_pin);
}