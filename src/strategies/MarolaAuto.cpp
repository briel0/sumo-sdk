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
    _velAvanco = VEL_ATAQUE_MAX;

    switch(cfg.search) {
        case BUSCA_LINHA:
            _atrasoAbrirAsaMs = 100;
            _bloqueiaAteAbrir = false;
            _velAvanco = VEL_BUSCA_LINHA;
            Serial.printf("[MAROLA] Busca SENSOR DE LINHA (avanca a %d%%).\n", VEL_BUSCA_LINHA);
            break;
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

    // Prioridade maxima sobre os motores: LINHA BRANCA. Vem antes do giro
    // inicial, da espera da asa e do avanco — nao adianta ganhar o alvo e
    // perder o dojo.
    bool leuEsq = _linhaEsq.temLinhaBranca();
    bool leuDir = _linhaDir.temLinhaBranca();

    if(leuEsq || leuDir) {
        // Reinicia o movimento sempre que ve a linha: fica preso no primeiro
        // passo (re) ate a linha sumir.
        if(leuEsq) {
            _player.play(Config::MACRO_FUGA_LINHA_ESQUERDA);
        }
        else if(leuDir) {
            _player.play(Config::MACRO_FUGA_LINHA_DIREITA);
        }
    }

    // Com a fuga de borda ou o giro inicial (ASA_DELAY_100_GIRO) tocando, o
    // player e' dono dos motores até a macro terminar — só então o fluxo
    // normal abaixo assume.
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

    // Igual ao LDR do Caipora: abaixo do limiar, algo bloqueou a luz — alvo
    // na cara, ataca com tudo. Qualquer um dos três LDRs serve de gatilho.
    if(_ldrEsq.readRaw() < LIMIAR_LDR || _ldrDir.readRaw() < LIMIAR_LDR || _ldrFront.readRaw() < LIMIAR_LDR) {
        motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_MAX);
        return;
    }

    // Auto simplificado: anda pra frente até o STOP do IR (botão 3, ver main.cpp).
    motores.setSpeed(_velAvanco, _velAvanco);
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
