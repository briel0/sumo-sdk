#include "SmokerAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

// Sequências de fuga ao detectar a linha branca da borda
static const MotionSequence MACRO_RECUO_ESQUERDA = MACRO(
    {-100, -100, 120}, // Dá ré com tudo
    {100, -100, 120}   // Gira pra direita pra fugir
);

static const MotionSequence MACRO_RECUO_DIREITA = MACRO(
    {-100, -100, 120}, // Dá ré com tudo
    {-100, 100, 120}   // Gira pra esquerda pra fugir
);

SmokerAuto::SmokerAuto()
    : _sensorEsq(Config::PIN_JS_ESQ), _sensorDir(Config::PIN_JS_DIR), _sensorFrontal(Config::PIN_JS_FRONT),
      _linhaEsq(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ),
      _linhaDir(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR) {}

void SmokerAuto::init() {
    _sensorEsq.init();
    _sensorDir.init();
    _sensorFrontal.init();
    _linhaEsq.init();
    _linhaDir.init();

    _ultimoLado = Direction::left;
    _player.stop();
}

void SmokerAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    // A arma ja foi resolvida na largada, pela flag que veio do site: aqui ela nao
    // muda mais de estado. O servo fica no WeaponSystem, que o AutoMode atualiza.
    (void)armas;

    // 1. Prioridade Máxima Absoluta: LINHA BRANCA
    bool leuEsq = _linhaEsq.temLinhaBranca();
    bool leuDir = _linhaDir.temLinhaBranca();

    if(leuEsq || leuDir) {
        // Reinicia o movimento sempre que vê a linha.
        // Fica preso no primeiro passo (ré) até a linha sumir!
        if(leuEsq) {
            _player.play(MACRO_RECUO_ESQUERDA);
        }
        else if(leuDir) {
            _player.play(MACRO_RECUO_DIREITA);
        }
    }

    // 2. Com a fuga tocando, ela e dona dos motores: nem busca nem ataque opinam.
    if(_player.isPlaying()) {
        _player.update(motores);
        return;
    }

    // 3. Snapshot unico dos JS40F — busca e ataque leem exatamente o mesmo frame.
    bool viuEsq = _sensorEsq.temAlvo();
    bool viuDir = _sensorDir.temAlvo();
    bool viuFrente = _sensorFrontal.temAlvo();

    if(viuEsq)
        _ultimoLado = Direction::left;
    else if(viuDir)
        _ultimoLado = Direction::right;

    // O frontal e o unico gatilho de ataque: os laterais so trazem o oponente pro
    // arco da frente, que e onde a rampa do servo pega ele. Os dois caminhos daqui
    // pra baixo sempre comandam os motores, entao o robo nunca fica com PWM velho
    // travado quando perde o alvo.
    if(viuFrente) {
        _ataque(motores, viuEsq, viuDir, viuFrente);
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
    // Cegueira total: gira pro ultimo lado onde alguem apareceu.
    if(_ultimoLado == Direction::right)
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    else
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
}

void SmokerAuto::_ataque(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {
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

String SmokerAuto::getSensorStatusJSON() {
    String json = "{";
    json += "\"Frontal (JS40F)\": " + String(_sensorFrontal.temAlvo() ? 1 : 0) + ", ";
    json += "\"Esquerda (JS40F)\": " + String(_sensorEsq.temAlvo() ? 1 : 0) + ", ";
    json += "\"Direita (JS40F)\": " + String(_sensorDir.temAlvo() ? 1 : 0) + ", ";
    json += "\"Linha Esq\": " + String(_linhaEsq.leituraRaw()) + ", ";
    json += "\"Linha Dir\": " + String(_linhaDir.leituraRaw());
    json += "}";
    return json;
}
