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

int LDR::getPercentage() const {
    int rawValue = analogRead(_pin);

    // Mapeia a resolução de 12-bits do ESP32 (0-4095) para 0-100%
    // Atenção: Se o teu circuito físico for invertido (LDR no GND em vez do VCC),
    // basta inverter os dois últimos parâmetros: map(rawValue, 0, 4095, 100, 0);
    return map(rawValue, 0, 4095, 0, 100);
}

bool LDR::isDark(int threshold) const {
    return analogRead(_pin) < threshold;
}