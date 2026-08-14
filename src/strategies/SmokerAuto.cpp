#include "strategies/SmokerAuto.hpp"
#include "hardware/Drive.hpp"
#include "hardware/HardwareCore.hpp"
#include "hardware/WeaponSystem.hpp"

void SmokerAuto::init(HardwareCore &hardware) {
    // Liga os emissores IR laterais para enxergar o adversário nas diagonais.
    hardware.setStealth(true);
    hardware.setWing(WingPosition::RETRACTED);
    _ultimoLado = Direction::left;
}

void SmokerAuto::autoEngage(Drive &motores, WeaponSystem &armas, HardwareCore &hardware) {
    (void)armas;

    // Borda do dohyo: linha branca sob qualquer sensor -> recuo imediato.
    if(hardware.lineLeft() || hardware.lineRight()) {
        motores.setSpeed(-VEL_ATAQUE_MAX, -VEL_ATAQUE_MAX);
        return;
    }

    bool viuEsq    = hardware.leftDetected();
    bool viuDir    = hardware.rightDetected();
    bool viuFrente = hardware.frontDetected();

    if(viuEsq) {
        _ultimoLado = Direction::left;
        hardware.setWing(WingPosition::LEFT);
    }
    else if(viuDir) {
        _ultimoLado = Direction::right;
        hardware.setWing(WingPosition::RIGHT);
    }

    if(viuFrente || (viuEsq && viuDir)) {
        _ataque(motores, hardware);
    }
    else {
        _busca(motores, hardware);
    }
}

void SmokerAuto::_busca(Drive &motores, HardwareCore &hardware) {
    if(hardware.frontDetected()) {
        return; // será tratado no próximo frame pelo ataque
    }
    if(hardware.leftDetected()) {
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
        return;
    }
    if(hardware.rightDetected()) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
        return;
    }
    // Cegueira total: gira para o último lado visto
    if(_ultimoLado == Direction::right)
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    else
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
}

void SmokerAuto::_ataque(Drive &motores, HardwareCore &hardware) {
    bool viuEsq    = hardware.leftDetected();
    bool viuDir    = hardware.rightDetected();
    bool viuFrente = hardware.frontDetected();

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
