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

    _tempoEntradaCombate = 0;
    _player.stop();
}

void MarolaAuto::configure(const AutoStrategy &cfg) {
    _giroInicialMacro = nullptr;

    switch(cfg.search) {
        case ASA_DELAY_500:
            _atrasoAbrirAsaMs = 500;
            _bloqueiaAteAbrir = true;
            Serial.println("[MAROLA] Asa abre durante os 500ms (parado até o fim da espera).");
            break;
        case ASA_DELAY_100_GIRO:
            _atrasoAbrirAsaMs = 100;
            _bloqueiaAteAbrir = false;
            _giroInicialMacro =
                cfg.direction == 'D' ? &Config::MACRO_GIRO_INICIAL_DIREITA : &Config::MACRO_GIRO_INICIAL_ESQUERDA;
            Serial.printf("[MAROLA] Asa abre durante os 100ms, com giro inicial pra %s antes de avançar.\n",
                          cfg.direction == 'D' ? "direita" : "esquerda");
            break;
        case ASA_DELAY_100:
        default:
            _atrasoAbrirAsaMs = 100;
            _bloqueiaAteAbrir = false;
            Serial.println("[MAROLA] Asa abre durante os 100ms (anda desde o início).");
            break;
    }
}

void MarolaAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    armas.update();

    if(_tempoEntradaCombate == 0) {
        _tempoEntradaCombate = millis();
        if(_giroInicialMacro != nullptr) {
            _player.play(*_giroInicialMacro);
        }
    }

    // A asa abre logo de cara em qualquer modo — inclusive durante o giro
    // inicial — o delay escolhido no site e' só o tempo que ela física leva
    // pra terminar de abrir, não um atraso pra começar a abrir.
    if(!armas.isDeployed()) {
        armas.deploy();
    }

    // Giro inicial (ASA_DELAY_100_GIRO) tem prioridade total sobre os motores
    // até a macro terminar — só então o fluxo normal abaixo assume.
    if(_player.isPlaying()) {
        _player.update(motores);
        return;
    }

    unsigned long decorrido = millis() - _tempoEntradaCombate;

    // Só o modo 500ms é bloqueante pro movimento: fica parado até o fim da
    // janela e só então sai andando. 100ms já anda desde o primeiro frame.
    if(_bloqueiaAteAbrir && decorrido < _atrasoAbrirAsaMs) {
        motores.setSpeed(0, 0);
        return;
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
