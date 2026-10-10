#include "LDR.hpp"

// O construtor usa a Lista de Inicialização para atribuir o pino
LDR::LDR(uint8_t pin) : _pin(pin) {}

void LDR::init() {
    // Configura o pino para leitura.
    // Em pinos ADC do ESP32, o pinMode é muitas vezes opcional,
    // mas mantê-lo garante estabilidade e clareza na arquitetura.
    pinMode(_pin, INPUT);
}

int LDR::readRaw() const {
    return analogRead(_pin);
}
