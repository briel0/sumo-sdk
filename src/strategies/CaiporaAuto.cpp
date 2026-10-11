#include "CaiporaAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

CaiporaAuto::CaiporaAuto()
    : _ldr(PIN_LDR), _vlFrenteEsq(PIN_XSHUT_FRENTE_ESQ, 0x30), _vlLateralEsq(PIN_XSHUT_LATERAL_ESQ, 0x31),
      _vlFrenteDir(PIN_XSHUT_FRENTE_DIR, 0x32), _vlLateralDir(PIN_XSHUT_LATERAL_DIR, 0x33) {}

void CaiporaAuto::init() {
    _ldr.init();

    _vlFrenteEsq.disable();
    _vlLateralEsq.disable();
    _vlFrenteDir.disable();
    _vlLateralDir.disable();
    delay(20);

    _frenteEsqOk = _vlFrenteEsq.init();
    _lateralEsqOk = _vlLateralEsq.init();
    _frenteDirOk = _vlFrenteDir.init();
    _lateralDirOk = _vlLateralDir.init();

    if(!_frenteEsqOk)
        Serial.println("[CAIPORA] AVISO: VL53L0X Frente Esq nao subiu.");
    if(!_lateralEsqOk)
        Serial.println("[CAIPORA] AVISO: VL53L0X Lateral Esq nao subiu. Busca vai ignorar esse lado.");
    if(!_frenteDirOk)
        Serial.println("[CAIPORA] AVISO: VL53L0X Frente Dir nao subiu.");
    if(!_lateralDirOk)
        Serial.println("[CAIPORA] AVISO: VL53L0X Lateral Dir nao subiu. Busca vai ignorar esse lado.");

    _ultimoLado = Direction::left;
    _player.stop();
}

void CaiporaAuto::configure(const AutoStrategy &cfg) {
    // A busca comeca pelo lado OPOSTO ao da estrategia inicial escolhida no
    // site: quem sai pra direita termina com o adversario a esquerda, e
    // vice-versa. Sem lado escolhido ('X') o saque toca a esquerda, entao a
    // busca comeca pela direita.
    _ultimoLado = cfg.direction == 'D' ? Direction::left : Direction::right;

    // Id desconhecido cai na RAPIDA pelo default — nunca deixa o robô sem busca.
    _velSemAlvo = 0;

    switch(cfg.search) {
        case BUSCA_LENTA_OFENSIVA:
            _buscaAtual = &CaiporaAuto::_buscaLenta;
            _velSemAlvo = VEL_OFENSIVA;
            Serial.println("[CAIPORA] Busca LENTA OFENSIVA.");
            break;
        case BUSCA_RAPIDA_OFENSIVA:
            _buscaAtual = &CaiporaAuto::_buscaRapida;
            _velSemAlvo = VEL_OFENSIVA;
            Serial.println("[CAIPORA] Busca RAPIDA OFENSIVA.");
            break;
        case BUSCA_LENTA:
            _buscaAtual = &CaiporaAuto::_buscaLenta;
            Serial.println("[CAIPORA] Busca LENTA DEFENSIVA.");
            break;
        case BUSCA_RAPIDA:
        default:
            _buscaAtual = &CaiporaAuto::_buscaRapida;
            Serial.println("[CAIPORA] Busca RAPIDA DEFENSIVA.");
            break;
    }
}

void CaiporaAuto::autoEngage(Drive &motores, WeaponSystem &armas) {

    // 1. Se o MotionPlayer estiver tocando alguma macro (ex: largada), ele assume o controle
    if(_player.isPlaying()) {
        _player.update(motores);
        return;
    }

    unsigned long agora = millis();
    if(agora - _ultimaLeituraToF >= INTERVALO_TOF_MS) {
        _ultimaLeituraToF = agora;
        _rawFrenteEsq = _frenteEsqOk ? _vlFrenteEsq.leituraRaw() : SEM_LEITURA_MM;
        _rawLateralEsq = _lateralEsqOk ? _vlLateralEsq.leituraRaw() : SEM_LEITURA_MM;
        _rawFrenteDir = _frenteDirOk ? _vlFrenteDir.leituraRaw() : SEM_LEITURA_MM;
        _rawLateralDir = _lateralDirOk ? _vlLateralDir.leituraRaw() : SEM_LEITURA_MM;
    }

    // 3. LDR faz o papel do JS40F frontal do Arruela: abaixo do limiar, algo
    // bloqueou a luz de cima — alvo na cara.
    int rawLdr = _ldr.readRaw();

    (this->*_buscaAtual)(motores, _rawFrenteEsq, _rawLateralEsq, _rawFrenteDir, _rawLateralDir, rawLdr);
}

void CaiporaAuto::_buscaRapida(Drive &motores, uint16_t rawFrenteEsq, uint16_t rawLateralEsq, uint16_t rawFrenteDir,
                               uint16_t rawLateralDir, int rawLdr) {

    if(rawLdr < LIMIAR_LDR) {
        motores.setSpeed(VEL_ATAQUE, VEL_ATAQUE);
        return;
    }

    if(rawFrenteEsq <= 500 && rawFrenteDir <= 500) {
        motores.setSpeed(50, 50);
        return;
    }

    if(rawFrenteEsq <= 500) {
        motores.setSpeed(-55, 55);
        _ultimoLado = Direction::left;
        return;
    }

    if(rawFrenteDir <= 500) {
        motores.setSpeed(55, -55);
        _ultimoLado = Direction::right;
        return;
    }

    if(_ultimoLado == Direction::right) {
        motores.setSpeed(50, -50);
    }
    else {
        motores.setSpeed(-50, 50);
    }

    motores.setSpeed(_velSemAlvo, _velSemAlvo);
}

void CaiporaAuto::_buscaLenta(Drive &motores, uint16_t rawFrenteEsq, uint16_t rawLateralEsq, uint16_t rawFrenteDir,
                              uint16_t rawLateralDir, int rawLdr) {

    if(rawLdr < LIMIAR_LDR) {
        motores.setSpeed(VEL_ATAQUE, VEL_ATAQUE);
        return;
    }

    if(rawFrenteEsq <= 500 && rawFrenteDir <= 500) {
        motores.setSpeed(35, 35);
        return;
    }

    if(rawFrenteEsq <= 500) {
        motores.setSpeed(-55, 55);
        _ultimoLado = Direction::left;
        return;
    }

    if(rawFrenteDir <= 500) {
        motores.setSpeed(55, -55);
        _ultimoLado = Direction::right;
        return;
    }

    if(_ultimoLado == Direction::right) {
        motores.setSpeed(50, -50);
    }
    else {
        motores.setSpeed(-50, 50);
    }

    motores.setSpeed(_velSemAlvo, _velSemAlvo);
}

String CaiporaAuto::getSensorStatusJSON() {
    // -1 pra sensor que falhou no init() — sem isso o painel de bancada (lido
    // a cada 150ms por AutoMode::run(), ver o comentário lá sobre o portal
    // "entrando e saindo") ia bater o mesmo timeout de 500ms num sensor morto.
    String json = "{";
    json += "\"LDR\": " + String(_ldr.readRaw()) + ", ";
    json += "\"VL Frente Esq (mm)\": " + String(_frenteEsqOk ? (int)_vlFrenteEsq.leituraRaw() : -1) + ", ";
    json += "\"VL Lat Esq (mm)\": " + String(_lateralEsqOk ? (int)_vlLateralEsq.leituraRaw() : -1) + ", ";
    json += "\"VL Frente Dir (mm)\": " + String(_frenteDirOk ? (int)_vlFrenteDir.leituraRaw() : -1) + ", ";
    json += "\"VL Lat Dir (mm)\": " + String(_lateralDirOk ? (int)_vlLateralDir.leituraRaw() : -1);
    json += "}";
    return json;
}
