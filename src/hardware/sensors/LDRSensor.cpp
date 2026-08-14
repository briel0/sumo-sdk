#include "hardware/sensors/LDRSensor.hpp"

LDRSensor::LDRSensor(uint8_t pin, uint16_t threshold, uint8_t windowSize)
    : _pin(pin), _threshold(threshold), _window(constrain(windowSize, 1, MAX_WINDOW)) {}

void LDRSensor::init() {
    pinMode(_pin, INPUT);

    // Prime o filtro com a leitura atual em toda a janela, evitando um transitório
    // frio que dispararia belowThreshold() indevidamente nas primeiras iterações.
    uint16_t inicial = analogRead(_pin);
    for(uint8_t i = 0; i < _window; i++) {
        _samples[i] = inicial;
    }
    _index    = 0;
    _sum      = (uint32_t)inicial * _window;
    _filtered = inicial;
}

void LDRSensor::update() {
    // Média móvel de soma corrente: tira a amostra mais antiga, injeta a nova.
    _sum -= _samples[_index];
    uint16_t nova   = analogRead(_pin);
    _samples[_index] = nova;
    _sum += nova;

    _index    = (_index + 1) % _window;
    _filtered = (uint16_t)(_sum / _window);
}

uint16_t LDRSensor::filtered() const {
    return _filtered;
}

bool LDRSensor::belowThreshold() const {
    return _filtered < _threshold;
}
