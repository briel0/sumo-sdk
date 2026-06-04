#include "StatusLed.hpp"
#include <Arduino.h>

void StatusLed::init(int debugPin, int dataPin, int numLeds) {
    _debugPin    = debugPin;
    _numLeds     = (numLeds <= MAX_LEDS) ? numLeds : MAX_LEDS;
    _currentStep = 0;

    pinMode(_debugPin, OUTPUT);
    digitalWrite(_debugPin, LOW);

    // FastLED exige que o pino de dados seja conhecido em compile time via template.
    // O workaround padrão para pino dinâmico é usar addLeds com o pino fixado
    // no perfil via constexpr e chamado de main. Aqui usamos o pino 33 diretamente
    // conforme a especificação do projeto.
    FastLED.addLeds<WS2812B, 33, GRB>(_leds, MAX_LEDS);
    FastLED.setBrightness(80);

    setAll(CRGB::Red);
}

void StatusLed::confirmStep() {
    if(_currentStep >= _numLeds) {
        return;
    }

    _leds[_currentStep] = CRGB::Green;
    FastLED.show();
    _currentStep++;

    delay(300);
}

void StatusLed::setAll(CRGB color) {
    for(int i = 0; i < _numLeds; i++) {
        _leds[i] = color;
    }
    FastLED.show();
}

void StatusLed::blinkDebug(int times, int intervalMs) {
    for(int i = 0; i < times; i++) {
        digitalWrite(_debugPin, HIGH);
        delay(intervalMs);
        digitalWrite(_debugPin, LOW);
        delay(intervalMs);
    }
}

void StatusLed::setState(CRGB color) {
    bool changed = false;
    for(int i = 0; i < _numLeds; i++) {
        if(_leds[i] != color) {
            _leds[i] = color;
            changed = true;
        }
    }
    if(changed) FastLED.show();
}

void StatusLed::heartbeat() {
    unsigned long now = millis();
    if(now - _lastHeartbeatMs < 20) return; // ~50fps
    _lastHeartbeatMs = now;

    uint8_t brilho = beatsin8(40, 10, 200);

    for(int i = 0; i < _numLeds; i++) {
        _leds[i] = CRGB(0, brilho, 0);
    }
    FastLED.show();
}

void StatusLed::pairingWave() {
    unsigned long now = millis();
    if(now - _lastWaveMs < 120) return; // velocidade da onda: 1 LED a cada 120ms
    _lastWaveMs = now;

    // Restaura o LED anterior para verde
    int prev = (_waveIndex - 1 + _numLeds) % _numLeds;
    _leds[prev] = CRGB::Green;

    // Acende o LED atual em laranja
    _leds[_waveIndex] = CRGB::Orange;
    FastLED.show();

    _waveIndex = (_waveIndex + 1) % _numLeds;
}

void StatusLed::strategyWave() {
    unsigned long now = millis();
    if(now - _lastWaveMs < 120) return;
    _lastWaveMs = now;

    // Restaura o LED anterior para verde
    _leds[_waveIndex] = CRGB::Green;

    // Avança ou recua o índice dentro da faixa 1–4
    if(_pingpongForward) {
        _waveIndex++;
        if(_waveIndex >= 4) {
            _waveIndex = 4;
            _pingpongForward = false;
        }
    } else {
        _waveIndex--;
        if(_waveIndex <= 1) {
            _waveIndex = 1;
            _pingpongForward = true;
        }
    }

    // Acende o LED atual em laranja
    _leds[_waveIndex] = CRGB::Orange;
    FastLED.show();
}