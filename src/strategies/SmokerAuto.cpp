#include "SmokerAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

SmokerAuto::SmokerAuto()
    : _sensorEsq(Config::PIN_JS_ESQ), _sensorDir(Config::PIN_JS_DIR), _sensorFrontal(Config::PIN_JS_FRONT),
      _linhaEsq(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD),
      _linhaDir(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD) {}

void SmokerAuto::init() {
    _sensorEsq.init();
    _sensorDir.init();
    _sensorFrontal.init();
    _linhaEsq.init();
    _linhaDir.init();
    _ultimoLado = Direction::left;
}

void SmokerAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    if(_linhaEsq.temLinhaBranca() || _linhaDir.temLinhaBranca()) {
        motores.setSpeed(-VEL_ATAQUE_MAX, -VEL_ATAQUE_MAX);
        return;
    }

    bool viuEsq = _sensorEsq.temAlvo();
    bool viuDir = _sensorDir.temAlvo();
    bool viuFrente = _sensorFrontal.temAlvo();

    if(viuEsq)
        _ultimoLado = Direction::left;
    else if(viuDir)
        _ultimoLado = Direction::right;

    if(viuFrente || (viuEsq && viuDir)) {
        _ataque(motores);
    }
    else if(viuEsq || viuDir) {
        _busca(motores); // lateral — centraliza antes de atacar
    }
    else {
        _busca(motores); // cegueira total
    }
}

void SmokerAuto::_busca(Drive &motores) {
    if(_sensorFrontal.temAlvo()) {
        return; // será tratado no próximo frame pelo ataque
    }
    if(_sensorEsq.temAlvo()) {
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
        return;
    }
    if(_sensorDir.temAlvo()) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
        return;
    }
    // Cegueira total: gira para o último lado visto
    if(_ultimoLado == Direction::right)
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    else
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
}

void SmokerAuto::_ataque(Drive &motores) {
    bool viuEsq = _sensorEsq.temAlvo();
    bool viuDir = _sensorDir.temAlvo();
    bool viuFrente = _sensorFrontal.temAlvo();

    if(!viuFrente && !viuEsq && !viuDir)
        return; // perdeu contato

    if(viuFrente) {
        motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_MAX);
        return;
    }
    if(viuEsq && !viuDir) {
        motores.setSpeed(VEL_ATAQUE_REDUZIDA, VEL_ATAQUE_MAX);
        return;
    }
    if(viuDir && !viuEsq) {
        motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_REDUZIDA);
        return;
    }
    motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_MAX);
}