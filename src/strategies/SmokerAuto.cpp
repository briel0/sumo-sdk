#include "SmokerAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

SmokerAuto::SmokerAuto()
    : _sensorEsq(Config::PIN_JS_ESQ), _sensorDir(Config::PIN_JS_DIR), _sensorFrontal(Config::PIN_JS_FRONT),
      _irEsq(Config::IR_ESQ), _irDir(Config::IR_DIR), _irFrontal(Config::IR_FRONT),
      _linhaEsq(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ),
      _linhaDir(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR) {}

void SmokerAuto::init() {
    _sensorEsq.init();
    _sensorDir.init();
    _sensorFrontal.init();
    _irEsq.init();
    _irDir.init();
    _irFrontal.init();
    _linhaEsq.init();
    _linhaDir.init();

    _ultimoLado = Direction::left;
    _player.stop();
}

void SmokerAuto::configure(const AutoStrategy &cfg) {
    // Id desconhecido cai na PADRAO pelo default — nunca deixa o robô sem busca.
    switch(cfg.search) {
        case BUSCA_IR:
            _buscaIr = true;
            digitalWrite(Config::PIN_JS_POWER, LOW);
            Serial.println("[SMOKER] Busca IR.");
            break;
        case BUSCA_PADRAO:
        default:
            _buscaIr = false;
            digitalWrite(Config::PIN_JS_POWER, HIGH);
            Serial.println("[SMOKER] Busca PADRAO.");
            break;
    }
}

void SmokerAuto::autoEngage(Drive &motores, WeaponSystem &armas) {

    armas.update();

    // Prioridade maxima absoluta: LINHA BRANCA. Vem antes de qualquer busca —
    // nao adianta ganhar o alvo e perder o dojo.
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

    // Com a fuga tocando, ela e dona dos motores: nem busca nem ataque opinam.
    if(_player.isPlaying()) {
        _player.update(motores);
        return;
    }

    bool viuEsq = _sensorEsq.temAlvo();
    bool viuDir = _sensorDir.temAlvo();
    bool viuFrente = _sensorFrontal.temAlvo();

    bool irEsq = _irEsq.temSinal();
    bool irDir = _irDir.temSinal();
    bool irFrente = _irFrontal.temSinal();

    // BUSCA IR: os JS40F estão sem alimentação, então quem enxerga são os IRs.
    if(_buscaIr) {
        viuEsq = irEsq;
        viuDir = irDir;
        viuFrente = irFrente;
    }

    if(viuEsq)
        _ultimoLado = Direction::left;
    else if(viuDir)
        _ultimoLado = Direction::right;

    // Frontal manda: alvo na frente ataca reto, sem girar.
    if(viuFrente) {
        _ataque(motores);
        return;
    }

    _busca(motores, viuEsq, viuDir);
}

void SmokerAuto::_busca(Drive &motores, bool viuEsq, bool viuDir) {
    if(viuEsq) {
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
        return;
    }
    if(viuDir) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
        return;
    }


    if(_ultimoLado == Direction::right) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    }
    else {
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
    }
}

void SmokerAuto::_ataque(Drive &motores) {
    motores.setSpeed(VEL_ATAQUE_MAX, VEL_ATAQUE_MAX);
}

String SmokerAuto::getSensorStatusJSON() {
    String json = "{";
    json += "\"Frontal (JS40F)\": " + String(_sensorFrontal.temAlvo() ? 1 : 0) + ", ";
    json += "\"Esquerda (JS40F)\": " + String(_sensorEsq.temAlvo() ? 1 : 0) + ", ";
    json += "\"Direita (JS40F)\": " + String(_sensorDir.temAlvo() ? 1 : 0) + ", ";
    json += "\"IR Frontal (TSSP4038)\": " + String(_irFrontal.temSinal() ? 1 : 0) + ", ";
    json += "\"IR Esquerda (TSSP4038)\": " + String(_irEsq.temSinal() ? 1 : 0) + ", ";
    json += "\"IR Direita (TSSP4038)\": " + String(_irDir.temSinal() ? 1 : 0) + ", ";
    json += "\"Linha Esq\": " + String(_linhaEsq.leituraRaw()) + ", ";
    json += "\"Linha Dir\": " + String(_linhaDir.leituraRaw());
    json += "}";
    return json;
}
