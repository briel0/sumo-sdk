#include "ArruelaAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

// Sequencias de fuga ao detectar a linha branca da borda
static const MotionSequence MACRO_RECUO_ESQUERDA = MACRO(
    {-100, -100, 120}, // Da re com tudo
    {100, -100, 120}   // Gira pra direita pra fugir
);

static const MotionSequence MACRO_RECUO_DIREITA = MACRO(
    {-100, -100, 120}, // Da re com tudo
    {-100, 100, 120}   // Gira pra esquerda pra fugir
);

ArruelaAuto::ArruelaAuto()
    : _sensorEsq(Config::PIN_JS_ESQ), _sensorDir(Config::PIN_JS_DIR), _sensorFrontal(Config::PIN_JS_FRONT),
      _sensorDistancia(), _linhaEsq(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD_ESQ),
      _linhaDir(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD_DIR) {}

void ArruelaAuto::init() {
    _sensorEsq.init();
    _sensorDir.init();
    _sensorFrontal.init();
    _linhaEsq.init();
    _linhaDir.init();

    // O retorno importa: sem ele a falha do VL53L0X e silenciosa e a BUSCA_TOF
    // gira a luta inteira sem nunca atacar. O configure() consulta este flag.
    _toFOk = _sensorDistancia.init();
    if(!_toFOk) {
        Serial.println("[ARRUELA] AVISO: VL53L0X nao subiu. BUSCA POR DISTANCIA indisponivel.");
    }

    _ultimoLado = Direction::left;
    _ultimaLeituraToF = 0;
    _toFViuAlvo = false;
    _player.stop();
}

void ArruelaAuto::configure(const AutoStrategy &cfg) {
    // Cada id do site aponta para uma funcao de busca. Id desconhecido cai no
    // padrao pelo default — nunca deixa o robo sem busca.
    switch(cfg.search) {
        case BUSCA_LENTA:
            _buscaAtual = &ArruelaAuto::_buscaLenta;
            Serial.println("[ARRUELA] Busca LENTA.");
            break;
        case BUSCA_TOF:
            if(_toFOk) {
                _buscaAtual = &ArruelaAuto::_buscaToF;
                Serial.printf("[ARRUELA] Busca POR DISTANCIA (ataca abaixo de %umm).\n", LIMIAR_TOF_MM);
            }
            else {
                // Sem ToF esse modo nunca atacaria. Melhor lutar com a busca
                // padrao do que girar a luta inteira sem partir pra cima.
                _buscaAtual = &ArruelaAuto::_buscaPadrao;
                Serial.println("[ARRUELA] BUSCA POR DISTANCIA pedida sem VL53L0X. Caindo na PADRAO.");
            }
            break;
        case BUSCA_PADRAO:
        default:
            _buscaAtual = &ArruelaAuto::_buscaPadrao;
            Serial.println("[ARRUELA] Busca PADRAO.");
            break;
    }

    // Zera o estado do ToF pra uma busca nova nao herdar leitura da anterior.
    _ultimaLeituraToF = 0;
    _toFViuAlvo = false;
}

void ArruelaAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    // 1. Prioridade maxima absoluta: LINHA BRANCA. Vem antes de qualquer busca —
    // nao adianta ganhar o alvo e perder o dojo.
    bool leuEsq = _linhaEsq.temLinhaBranca();
    bool leuDir = _linhaDir.temLinhaBranca();

    if(leuEsq || leuDir) {
        // Reinicia o movimento sempre que ve a linha: fica preso no primeiro
        // passo (re) ate a linha sumir.
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

    // 3. Snapshot único — todos os métodos usam o mesmo estado

    bool viuEsq = _sensorEsq.temAlvo();
    bool viuDir = _sensorDir.temAlvo();
    bool viuFrente = _sensorFrontal.temAlvo();

    if(viuEsq)
        _ultimoLado = Direction::left;
    else if(viuDir)
        _ultimoLado = Direction::right;

    // Quem decide atacar e o modo de busca, nao o autoEngage: cada busca recebe o
    // frame completo e define seu proprio gatilho de ataque.
    (this->*_buscaAtual)(motores, viuEsq, viuDir, viuFrente);
}

void ArruelaAuto::_buscaPadrao(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {
    if(viuFrente) {
        _ataque(motores, viuEsq, viuDir, viuFrente);
        return;
    }
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

void ArruelaAuto::_buscaLenta(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {
    if(viuFrente) {
        _ataque(motores, viuEsq, viuDir, viuFrente);
        return;
    }
    if(viuEsq) {
        motores.setSpeed(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
        return;
    }
    if(viuDir) {
        motores.setSpeed(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
        return;
    }
    if(_ultimoLado == Direction::right)
        motores.setSpeed(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
    else
        motores.setSpeed(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
}

void ArruelaAuto::_buscaToF(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {
    // O JS40F frontal e deliberadamente ignorado aqui: neste modo quem autoriza o
    // ataque e o VL53L0X, e so ele.
    (void)viuFrente;

    // Leitura espacada. readRangeContinuousMillimeters() gira no I2C ate sair
    // amostra nova, entao ler todo frame prenderia o loop na cadencia do sensor.
    unsigned long agora = millis();
    if(agora - _ultimaLeituraToF >= INTERVALO_TOF_MS) {
        _ultimaLeituraToF = agora;
        _toFViuAlvo = _sensorDistancia.temOponente(LIMIAR_TOF_MM);
    }

    if(_toFViuAlvo) {
        // O ToF olha pra frente, entao alvo no alcance e alvo na cara: vai pra cima.
        _ataque(motores, viuEsq, viuDir, true);
        return;
    }

    // Fora do alcance: gira devagar procurando. Devagar de proposito — o cone do
    // VL53L0X e estreito e so e amostrado a cada INTERVALO_TOF_MS, entao girando
    // rapido o alvo atravessa o cone entre duas leituras e o gatilho nunca arma.
    // Os laterais seguem orientando o giro: eles trazem o oponente pro arco
    // frontal e o ToF confirma a distancia.
    if(viuEsq) {
        motores.setSpeed(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
        return;
    }
    if(viuDir) {
        motores.setSpeed(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
        return;
    }
    if(_ultimoLado == Direction::right)
        motores.setSpeed(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
    else
        motores.setSpeed(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
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

String ArruelaAuto::getSensorStatusJSON() {
    String json = "{";
    json += "\"Frontal (JS40F)\": " + String(_sensorFrontal.temAlvo() ? 1 : 0) + ", ";
    json += "\"Esquerda (JS40F)\": " + String(_sensorEsq.temAlvo() ? 1 : 0) + ", ";
    json += "\"Direita (JS40F)\": " + String(_sensorDir.temAlvo() ? 1 : 0) + ", ";
    json += "\"Distancia (ToF)\": " + String(_sensorDistancia.leituraRaw()) + ", ";
    json += "\"Linha Esq\": " + String(_linhaEsq.leituraRaw()) + ", ";
    json += "\"Linha Dir\": " + String(_linhaDir.leituraRaw());
    json += "}";
    return json;
}