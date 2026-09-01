#include "strategies/FuegoAuto.hpp"
#include "hardware/Drive.hpp"
#include "hardware/HardwareCore.hpp"
#include "hardware/WeaponSystem.hpp"
#include <Arduino.h>

void FuegoAuto::init(HardwareCore &hardware) {
    // Emissores IR laterais LIGADOS: sem sensor frontal, os dois JSumo são a
    // única detecção do Fuego, e o HardwareCore só usa o JSumo com o emissor
    // ligado (desligado ele cai no IR puro, passivo e mais curto). Modo furtivo
    // aqui custaria a única visão que o robô tem.
    hardware.setStealth(true);

    // Asa recolhida até a largada — quem a abre é o primeiro frame de combate.
    hardware.setWing(WingPosition::RETRACTED);

    _ultimoLado    = Direction::left;
    _asaAberta     = false;
    _rampaAtacando = false;
}

void FuegoAuto::autoEngage(Drive &motores, WeaponSystem &armas, HardwareCore &hardware) {
    (void)armas;

    // Início da luta: asa aberta à esquerda, e fica lá o resto do combate.
    if(!_asaAberta) {
        hardware.setWing(WingPosition::LEFT);
        _asaAberta = true;
        Serial.println("[FUEGO] Combate iniciado: asa aberta à esquerda.");
    }

    // Prioridade máxima: LDR acusou oponente embarcado na rampa -> empurra.
    // Enquanto ele estiver lá, nenhuma outra lógica deste método roda.
    if(hardware.opponentOnRamp()) {
        if(!_rampaAtacando) {
            Serial.println("[FUEGO] LDR: oponente na rampa -> EMPURRA.");
            _rampaAtacando = true;
        }
        motores.setSpeed(VEL_RAMPA, VEL_RAMPA);
        return;
    }
    if(_rampaAtacando) {
        Serial.println("[FUEGO] LDR: rampa livre.");
        _rampaAtacando = false;
    }

    // Sem leitura de linha branca: o Fuego não usa os sensores de borda em
    // combate (ver o cabeçalho da classe). Da rampa em diante é só busca/ataque.
    bool viuEsq = hardware.leftDetected();
    bool viuDir = hardware.rightDetected();

    if(viuEsq) {
        _ultimoLado = Direction::left;
    }
    else if(viuDir) {
        _ultimoLado = Direction::right;
    }

    if(viuEsq || viuDir) {
        _ataque(motores, viuEsq, viuDir);
    }
    else {
        _busca(motores);
    }
}

void FuegoAuto::_busca(Drive &motores) {
    // Busca simples: giro no próprio eixo pro último lado visto. Sem varredura
    // em zigue-zague nem busca linha — o robô só roda até um dos laterais pegar
    // o oponente, e aí o ataque assume.
    if(_ultimoLado == Direction::right) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    }
    else {
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
    }
}

void FuegoAuto::_ataque(Drive &motores, bool viuEsq, bool viuDir) {
    // Os dois laterais juntos = alvo centrado à frente (é o que substitui o
    // sensor frontal que este robô não tem): força bruta reta.
    if(viuEsq && viuDir) {
        motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_MAX);
        return;
    }

    // Um lateral só: arco fechando pra aquele lado — reduz a roda de dentro em
    // vez de girar parado, para avançar e alinhar ao mesmo tempo. Girar parado
    // aqui dependeria dos dois feixes se cruzarem pra sair do lugar.
    if(viuEsq) {
        motores.setSpeed(VEL_ATAQUE_REDUZIDA, VEL_ATAQUE_MAX);
        return;
    }
    motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_REDUZIDA);
}
