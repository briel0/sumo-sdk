#include "DiagnosticsPanel.hpp"

const CRGB DiagnosticsPanel::DETECT_COLOR    = CRGB::Green;
const CRGB DiagnosticsPanel::REST_COLOR      = CRGB::Red;
const CRGB DiagnosticsPanel::HIGHLIGHT_COLOR = CRGB::Blue;
const CRGB DiagnosticsPanel::OFF_COLOR       = CRGB::Black;

DiagnosticsPanel::DiagnosticsPanel(StatusLed &statusLed) : _statusLed(statusLed) {}

void DiagnosticsPanel::showSensorReadings(bool rampaLdr, bool linhaEsq, bool irEsq, bool irDir, bool linhaDir) {
    _statusLed.setLedRaw(0, rampaLdr ? DETECT_COLOR : REST_COLOR);
    _statusLed.setLedRaw(1, linhaEsq ? DETECT_COLOR : REST_COLOR);
    _statusLed.setLedRaw(2, irEsq ? DETECT_COLOR : REST_COLOR);
    _statusLed.setLedRaw(3, irDir ? DETECT_COLOR : REST_COLOR);
    _statusLed.setLedRaw(4, linhaDir ? DETECT_COLOR : REST_COLOR);
    _statusLed.push();
}

void DiagnosticsPanel::showMotionVector(MotorTestState state) {
    if(state == MotorTestState::PARADO) {
        // Regra especial: os 5 na cor de repouso, não de destaque.
        for(int i = 0; i < 5; i++) {
            _statusLed.setLedRaw(i, REST_COLOR);
        }
        _statusLed.push();
        return;
    }

    // Índices do vetor de movimento por estado (ver topologia no header).
    bool highlighted[5] = {false, false, false, false, false};
    switch(state) {
        case MotorTestState::FRENTE:
            highlighted[1] = highlighted[2] = highlighted[3] = highlighted[4] = true;
            break;
        case MotorTestState::TRAS:
            highlighted[0] = highlighted[2] = highlighted[3] = true;
            break;
        case MotorTestState::ESQUERDA:
            highlighted[0] = highlighted[1] = highlighted[2] = true;
            break;
        case MotorTestState::DIREITA:
            highlighted[0] = highlighted[3] = highlighted[4] = true;
            break;
        case MotorTestState::PARADO:
            break; // tratado acima
    }

    for(int i = 0; i < 5; i++) {
        _statusLed.setLedRaw(i, highlighted[i] ? HIGHLIGHT_COLOR : OFF_COLOR);
    }
    _statusLed.push();
}

void DiagnosticsPanel::showServoState(WingPosition pos) {
    bool lit[5] = {false, false, false, false, false};
    switch(pos) {
        case WingPosition::LEFT:
            lit[0] = lit[1] = true; // asa aberta para a esquerda
            break;
        case WingPosition::RIGHT:
            lit[3] = lit[4] = true; // asa aberta para a direita
            break;
        case WingPosition::RETRACTED:
            lit[2] = true; // recolhida (centro)
            break;
    }

    for(int i = 0; i < 5; i++) {
        _statusLed.setLedRaw(i, lit[i] ? HIGHLIGHT_COLOR : OFF_COLOR);
    }
    _statusLed.push();
}
