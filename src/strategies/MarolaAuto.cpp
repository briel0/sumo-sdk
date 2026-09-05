#include "MarolaAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

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

    _player.stop();
}

void MarolaAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    armas.update();

    // Arma o mais rápido possível ao entrar em combate.
    if(!armas.isDeployed()) {
        armas.deploy();
    }

    // Auto simplificado: anda pra frente até o STOP do IR (botão 3, ver main.cpp).
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
