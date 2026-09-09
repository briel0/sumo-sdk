#include "ArruelaAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"
#include <Arduino.h>

ArruelaAuto::ArruelaAuto()
    : _sensorEsq(Config::PIN_JS_ESQ), _sensorDir(Config::PIN_JS_DIR), _sensorFrontal(Config::PIN_JS_FRONT),
      _sensorDistancia() {}

void ArruelaAuto::init() {
    _sensorEsq.init();
    _sensorDir.init();
    _sensorFrontal.init();

    // O retorno importa: sem ele a falha do VL53L0X e silenciosa e a BUSCA_TOF
    // gira a luta inteira sem nunca atacar. O configure() consulta este flag.
    _toFOk = _sensorDistancia.init();
    if(!_toFOk) {
        Serial.println("[ARRUELA] AVISO: VL53L0X nao subiu. BUSCA POR DISTANCIA indisponivel.");
    }

    _ultimoLado = Direction::left;
    _ultimaLeituraToF = 0;
    _toFViuAlvo = false;
    _inicioGatoPreto = 0;
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
        case BUSCA_BALA_TENSA:
            if(_toFOk) {
                _buscaAtual = &ArruelaAuto::_buscaBalaTensa;
                Serial.printf("[ARRUELA] Busca BALA TENSA (curvinha abaixo de %umm).\n", LIMIAR_TOF_MM);
            }
            else {
                // Sem ToF esse modo nunca dispararia a curvinha. Melhor lutar
                // com a busca padrao do que girar a luta inteira sem reagir.
                _buscaAtual = &ArruelaAuto::_buscaPadrao;
                Serial.println("[ARRUELA] BALA TENSA pedida sem VL53L0X. Caindo na PADRAO.");
            }
            break;
        case BUSCA_GATO_PRETO:
            _buscaAtual = &ArruelaAuto::_buscaGatoPreto;
            if(!_toFOk) {
                Serial.println("[ARRUELA] GATO PRETO sem VL53L0X: só alinha e cai pra PADRAO depois da janela.");
            }
            Serial.printf("[ARRUELA] Busca GATO PRETO (%lums, ataca abaixo de %umm) -> PADRAO.\n",
                          DURACAO_GATO_PRETO_MS, LIMIAR_GATO_PRETO_MM);
            break;
        case BUSCA_PADRAO:
        default:
            _buscaAtual = &ArruelaAuto::_buscaPadrao;
            Serial.println("[ARRUELA] Busca PADRAO.");
            break;
    }

    // Zera o estado do ToF e do gato preto pra uma busca nova nao herdar
    // leitura/contagem da anterior.
    _ultimaLeituraToF = 0;
    _toFViuAlvo = false;
    _inicioGatoPreto = 0;
}

void ArruelaAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    // Liga o transistor dos JS40F só agora — ficou desligado de propósito
    // durante a estratégia inicial (ver main.cpp). Chamada redundante frame a
    // frame de propósito: é um digitalWrite, custa nada, e dispensa flag de
    // "já liguei".
    digitalWrite(Config::PIN_JS_POWER, HIGH);

    // 1. Com a fuga tocando, ela e dona dos motores: nem busca nem ataque opinam.
    if(_player.isPlaying()) {
        _player.update(motores);
        return;
    }

    // 2. Snapshot único — todos os métodos usam o mesmo estado

    bool viuEsq = _sensorEsq.temAlvo();
    bool viuDir = _sensorDir.temAlvo();
    bool viuFrente = _sensorFrontal.temAlvo();

    if(viuEsq) {
        _ultimoLado = Direction::left;
    }
    else if(viuDir) {
        _ultimoLado = Direction::right;
    }

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
    if(_ultimoLado == Direction::right) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    }
    else {
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
    }
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
    if(_ultimoLado == Direction::right) {
        motores.setSpeed(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
    }
    else {
        motores.setSpeed(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
    }
}

void ArruelaAuto::_buscaToF(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {

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
    if(_ultimoLado == Direction::right) {
        motores.setSpeed(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
    }
    else {
        motores.setSpeed(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
    }
}

void ArruelaAuto::_buscaBalaTensa(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {
    (void)viuFrente;

    // Mesma leitura espacada da BUSCA_TOF — reaproveita o cache
    // _ultimaLeituraToF/_toFViuAlvo, nao le o sensor de novo aqui.
    unsigned long agora = millis();
    if(agora - _ultimaLeituraToF >= INTERVALO_TOF_MS) {
        _ultimaLeituraToF = agora;
        _toFViuAlvo = _sensorDistancia.temOponente(LIMIAR_TOF_MM);
    }

    if(_toFViuAlvo) {
        // Em vez de partir reto pra cima, dispara uma curvinha pro ultimo
        // lado conhecido. autoEngage() confere _player.isPlaying() antes de
        // chamar a busca de novo, entao a partir daqui quem dirige os
        // motores e' o MotionPlayer ate a macro terminar.
        const MotionSequence &curvinha =
            _ultimoLado == Direction::right ? Config::MACRO_CURVINHA_DIREITA : Config::MACRO_CURVINHA_ESQUERDA;
        _player.play(curvinha);
        Serial.println("[ARRUELA] BALA TENSA: VL confirmou alvo perto. Disparando curvinha.");
        return;
    }

    // Fora do alcance: mesma varredura das outras buscas — laterais orientam
    // o giro, cegueira total gira pro ultimo lado conhecido.
    if(viuEsq) {
        motores.setSpeed(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
        return;
    }
    if(viuDir) {
        motores.setSpeed(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
        return;
    }
    if(_ultimoLado == Direction::right) {
        motores.setSpeed(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
    }
    else {
        motores.setSpeed(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
    }
}

void ArruelaAuto::_buscaGatoPreto(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {

    (void)viuFrente;

    if(_inicioGatoPreto == 0) {
        _inicioGatoPreto = millis();
    }

    // Leitura espacada do ToF, mesma cadencia da BUSCA_TOF.
    unsigned long agora = millis();
    if(agora - _ultimaLeituraToF >= INTERVALO_TOF_MS) {
        _ultimaLeituraToF = agora;
        _toFViuAlvo = _sensorDistancia.temOponente(LIMIAR_GATO_PRETO_MM);
    }

    if(_toFViuAlvo) {
        _buscaAtual = &ArruelaAuto::_buscaPadrao;
        _buscaPadrao(motores, viuEsq, viuDir, viuFrente);
        return;
    }

    if(millis() - _inicioGatoPreto >= DURACAO_GATO_PRETO_MS) {
        _buscaAtual = &ArruelaAuto::_buscaPadrao;
        _buscaPadrao(motores, viuEsq, viuDir, viuFrente);
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
    if(_ultimoLado == Direction::right) {
        motores.setSpeed(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
    }
    else {
        motores.setSpeed(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
    }
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
    json += "\"Distancia (ToF)\": " + String(_sensorDistancia.leituraRaw());
    json += "}";
    return json;
}