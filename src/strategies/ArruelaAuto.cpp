#include "ArruelaAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

ArruelaAuto::ArruelaAuto()
    : _sensorEsq(Config::PIN_JS_ESQ), _sensorDir(Config::PIN_JS_DIR), _sensorFrontal(Config::PIN_JS_FRONT) {}

void ArruelaAuto::init() {
    _sensorEsq.init();
    _sensorDir.init();
    _sensorFrontal.init();
    _ultimoLado = Direction::left;
}

void ArruelaAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    // Snapshot único — todos os métodos usam o mesmo estado
    bool viuEsq = _sensorEsq.temAlvo();
    bool viuDir = _sensorDir.temAlvo();
    bool viuFrente = _sensorFrontal.temAlvo();

    if(viuEsq)
        _ultimoLado = Direction::left;
    else if(viuDir)
        _ultimoLado = Direction::right;

    if(viuFrente) {
        _ataque(motores, viuEsq, viuDir, viuFrente);
    }
    else {
        _busca(motores, viuEsq, viuDir);
    }
}

void ArruelaAuto::_busca(Drive &motores, bool viuEsq, bool viuDir) {
    if(viuEsq) {
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
        return;
    }
    if(viuDir) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
        return;
    }
    if(_ultimoLado == Direction::right)
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    else
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
}

void ArruelaAuto::_ataque(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {
    if(!viuFrente && !viuEsq && !viuDir) {
        motores.setSpeed(0, 0);
        return;
    }
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