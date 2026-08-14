#include "StealthEmitter.hpp"

StealthEmitter::StealthEmitter(int pin) : _pin(pin) {}

void StealthEmitter::init() {
    pinMode(_pin, OUTPUT);
    disable();
}

void StealthEmitter::enable() {
    set(true);
}

void StealthEmitter::disable() {
    set(false);
}

void StealthEmitter::set(bool on) {
    digitalWrite(_pin, on ? HIGH : LOW);
    _enabled = on;
}
