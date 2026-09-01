#include "BootModeSelector.hpp"
#include "BootModeStore.hpp"
#include "Config.hpp"
#include "StatusLED.hpp"
#include <Arduino.h>

namespace {
    // Prefixo comum das três senhas. O quarto dígito é que escolhe o modo.
    constexpr int PREFIXO[] = {4, 5, 6};

    constexpr CRGB::HTMLColorCode COR_PROGRESSO = CRGB::Purple;

    CRGB corDoModo(RobotState mode) {
        switch(mode) {
            case RobotState::RC:
                return CRGB::Green;
            case RobotState::AUTO:
                return CRGB::Red;
            case RobotState::IDLE:
                return CRGB::Orange;
        }
        return CRGB::Black;
    }
}

void BootModeSelector::begin(RobotState compiledDefault) {
    _compiledDefault = compiledDefault;
    _progress = 0;
    _paintedProgress = -1;
    _owningLeds = false;
}

bool BootModeSelector::modeForFinalDigit(int digit, RobotState &out) {
    switch(digit) {
        case 7:
            out = RobotState::IDLE;
            return true;
        case 8:
            out = RobotState::AUTO;
            return true;
        case 9:
            out = RobotState::RC;
            return true;
        default:
            return false;
    }
}

bool BootModeSelector::feed(int digit) {
    if(digit <= 0) {
        return false;
    }

    // Senha já fechada: engolimos tudo até o reboot sair, pra nenhum aperto
    // atrasado engatilhar modo ou parada nos 2s de confirmação.
    if(_progress >= SEQUENCE_LENGTH) {
        return true;
    }

    // Último dígito: é ele que escolhe o modo.
    if(_progress == SEQUENCE_LENGTH - 1) {
        RobotState escolhido;
        if(!modeForFinalDigit(digit, escolhido)) {
            _progress = 0;
            return true;
        }
        _pendingMode = escolhido;
        _progress = SEQUENCE_LENGTH;
        _confirmedMs = millis();
        Serial.printf("[BOOT] Senha IR completa: próximo boot em %s.\n", BootModeStore::name(_pendingMode));
        return true;
    }

    if(digit != PREFIXO[_progress]) {
        // Fora de sequência. Se ainda nem tinha começado, o aperto não é nosso —
        // devolve pro tratamento normal (é assim que o 9 continua engatando RC no
        // IDLE). Se já estávamos digitando, derruba a senha e engole o dígito.
        if(_progress == 0) {
            return false;
        }
        _progress = 0;
        Serial.println("[BOOT] Dígito fora de sequência. Senha IR cancelada.");
        return true;
    }

    _progress++;
    return true;
}

void BootModeSelector::update(StatusLed &leds) {
    if(_progress == 0) {
        if(_owningLeds) {
            release(leds);
        }
        return;
    }

    if(!_owningLeds) {
        leds.lock();
        _owningLeds = true;
    }
    paint(leds);

    if(_progress >= SEQUENCE_LENGTH && millis() - _confirmedMs >= CONFIRM_HOLD_MS) {
        commit();
    }
}

void BootModeSelector::cancel(StatusLed &leds) {
    if(_progress >= SEQUENCE_LENGTH) {
        return; // reboot já contratado: a janela fechando não desfaz a escolha
    }
    if(_progress > 0) {
        Serial.println("[BOOT] Janela de configuração IR fechou. Senha cancelada.");
    }
    _progress = 0;
    if(_owningLeds) {
        release(leds);
    }
}

void BootModeSelector::paint(StatusLed &leds) {
    if(_paintedProgress == _progress) {
        return; // nada mudou: não custa um show() por frame à toa
    }
    _paintedProgress = _progress;

    for(int i = 0; i < Config::STATUS_LED_COUNT; i++) {
        leds.setLedRawLocked(i, CRGB::Black);
    }

    // Um LED roxo por dígito correto, começando no LED 2 (índice 1).
    for(int i = 0; i < _progress && i < SEQUENCE_LENGTH; i++) {
        leds.setLedRawLocked(1 + i, COR_PROGRESSO);
    }

    // Senha fechada: LED 1 (índice 0) na cor do modo escolhido.
    if(_progress >= SEQUENCE_LENGTH) {
        leds.setLedRawLocked(0, corDoModo(_pendingMode));
    }

    leds.pushLocked();
}

void BootModeSelector::release(StatusLed &leds) {
    _owningLeds = false;
    _paintedProgress = -1;
    leds.unlock();
    // Apaga o roxo aqui em vez de deixar pra animação seguinte: pairingWave e
    // strategyWave só repintam um LED por vez e demorariam a limpar a tira.
    leds.setAll(CRGB::Black);
}

void BootModeSelector::commit() {
    BootModeStore::save(_pendingMode, _compiledDefault);
    Serial.printf("[BOOT] Modo gravado (%s). Reiniciando...\n", BootModeStore::name(_pendingMode));
    Serial.flush();
    delay(50);
    ESP.restart();
}
