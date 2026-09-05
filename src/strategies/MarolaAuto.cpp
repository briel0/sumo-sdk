#include "MarolaAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

// Sequências de fuga ao detectar a linha branca da borda
static const MotionSequence MACRO_RECUO_ESQUERDA = MACRO(
    {-100, -100, 120}, // Dá ré com tudo por 200ms
    {100, -100, 120}   // Gira pra direita por 150ms pra fugir
);

static const MotionSequence MACRO_RECUO_DIREITA = MACRO(
    {-100, -100, 120}, // Dá ré com tudo por 200ms
    {-100, 100, 120}   // Gira pra esquerda por 150ms pra fugir
);

MarolaAuto::MarolaAuto()
    : _ldrEsq(Config::PIN_LDR_ESQ), _ldrDir(Config::PIN_LDR_DIR), _ldrFront(Config::PIN_LDR_FRONT),
      _linhaEsq(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ),
      _linhaDir(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR) {}

void MarolaAuto::init() {
    _ldrEsq.init();
    _ldrDir.init();
    _ldrFront.init();
    _linhaEsq.init();
    _linhaDir.init();

    _ultimoLado = Direction::left;
    _player.stop();
}

void MarolaAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    // 1. Prioridade Máxima Absoluta: LINHA BRANCA
    bool leuEsq = _linhaEsq.temLinhaBranca();
    bool leuDir = _linhaDir.temLinhaBranca();

    if(leuEsq || leuDir) {
        if(leuEsq) {
            _player.play(MACRO_RECUO_ESQUERDA);
        }
        else if(leuDir) {
            _player.play(MACRO_RECUO_DIREITA);
        }
    }

    if(_player.isPlaying()) {
        _player.update(motores);
        return;
    }

    // Leitura dos sensores LDR - deteccao do oponente acontece quando isDark() eh verdadeiro.
    bool viuEsq = _ldrEsq.isDark(Config::LDR_THRESHOLD);
    bool viuDir = _ldrDir.isDark(Config::LDR_THRESHOLD);
    bool viuFrente = _ldrFront.isDark(Config::LDR_THRESHOLD);

    if(viuEsq)
        _ultimoLado = Direction::left;
    else if(viuDir)
        _ultimoLado = Direction::right;

    if(viuFrente || viuDir || viuEsq) {
        _ataque(motores, viuEsq, viuDir, viuFrente);
    }
}

void MarolaAuto::_busca(Drive &motores, bool viuEsq, bool viuDir) {}

void MarolaAuto::_ataque(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {
    if(!viuFrente && !viuEsq && !viuDir) {
        motores.setSpeed(0, 0);
        return;
    }
    motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_MAX);
}

String MarolaAuto::getSensorStatusJSON() {
    String json = "{";
    json += "\"Linha Esq\": " + String(_linhaEsq.leituraRaw()) + ", ";
    json += "\"Linha Dir\": " + String(_linhaDir.leituraRaw()) + ", ";
    json += "\"LDR Esq\": " + String(_ldrEsq.readRaw()) + ", ";
    json += "\"LDR Dir\": " + String(_ldrDir.readRaw()) + ", ";
    json += "\"LDR Front\": " + String(_ldrFront.readRaw());
    json += "}";
    return json;
}
