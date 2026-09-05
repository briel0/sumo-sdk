#include "CaiporaAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

// Sequências de fuga ao detectar a linha branca da borda
static const MotionSequence MACRO_RECUO_ESQUERDA = MACRO(
    {-100, -100, 200}, // Dá ré com tudo por 200ms
    {100, -100, 150}   // Gira pra direita por 150ms pra fugir
);

static const MotionSequence MACRO_RECUO_DIREITA = MACRO(
    {-100, -100, 200}, // Dá ré com tudo por 200ms
    {-100, 100, 150}   // Gira pra esquerda por 150ms pra fugir
);

CaiporaAuto::CaiporaAuto()
    : _ldr(PIN_LDR), _vlFrenteEsq(PIN_XSHUT_FRENTE_ESQ, 0x30), _vlLateralEsq(PIN_XSHUT_LATERAL_ESQ, 0x31),
      _vlFrenteDir(PIN_XSHUT_FRENTE_DIR, 0x32), _vlLateralDir(PIN_XSHUT_LATERAL_DIR, 0x33),
      _linhaEsq(PIN_LINHA_ESQ, 2800), _linhaDir(PIN_LINHA_DIR, 2800) {}

void CaiporaAuto::init() {
    _ldr.init();
    _linhaEsq.init();
    _linhaDir.init();

    // Para evitar conflito I2C, precisamos garantir que todos os VL53L0X
    // comecem desligados antes de inicializá-los um a um.
    _vlFrenteEsq.disable();
    _vlLateralEsq.disable();
    _vlFrenteDir.disable();
    _vlLateralDir.disable();
    delay(20);

    // Agora liga e inicializa um por um (trocando o endereço)
    _vlFrenteEsq.init();
    _vlLateralEsq.init();
    _vlFrenteDir.init();
    _vlLateralDir.init();

    _ultimoLado = Direction::left;
    _player.stop();
}

void CaiporaAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    // 1. Prioridade Máxima Absoluta: LINHA BRANCA
    // O sensor de linha tem que "ganhar" e furar qualquer macro que o MotionPlayer esteja rodando (ex: ataque)
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

    // 2. Se o MotionPlayer estiver tocando algo (seja recuo, ou alguma macro de largada), ele assume o controle
    if(_player.isPlaying()) {
        _player.update(motores);
        return;
    }

    // 3. Busca por distância: só os dois VL frontais decidem. Leitura
    // espaçada (ver INTERVALO_TOF_MS) — sem isso os dois reads bloqueantes
    // travariam o loop e atrasariam a checagem da linha branca acima.
    unsigned long agora = millis();
    if(agora - _ultimaLeituraToF >= INTERVALO_TOF_MS) {
        _ultimaLeituraToF = agora;
        _viuFrenteEsq = _vlFrenteEsq.temOponente(LIMIAR_BUSCA_MM);
        _viuFrenteDir = _vlFrenteDir.temOponente(LIMIAR_BUSCA_MM);
    }

    _busca(motores, _viuFrenteEsq, _viuFrenteDir);
}

void CaiporaAuto::_busca(Drive &motores, bool viuFrenteEsq, bool viuFrenteDir) {
    // Os dois veem: alvo na cara, vai pra frente devagarinho.
    if(viuFrenteEsq && viuFrenteDir) {
        motores.setSpeed(VEL_BUSCA_FRENTE, VEL_BUSCA_FRENTE);
        return;
    }
    // Só a direita ve: gira pra direita devagarinho.
    if(viuFrenteDir) {
        _ultimoLado = Direction::right;
        motores.setSpeed(VEL_BUSCA_CUIDADOSA, -VEL_BUSCA_CUIDADOSA);
        return;
    }
    // Só a esquerda ve: gira pra esquerda devagarinho.
    if(viuFrenteEsq) {
        _ultimoLado = Direction::left;
        motores.setSpeed(-VEL_BUSCA_CUIDADOSA, VEL_BUSCA_CUIDADOSA);
        return;
    }
    // Nenhum dos dois ve nada: continua girando pro lado que viu por último.
    if(_ultimoLado == Direction::right)
        motores.setSpeed(VEL_BUSCA_CUIDADOSA, -VEL_BUSCA_CUIDADOSA);
    else
        motores.setSpeed(-VEL_BUSCA_CUIDADOSA, VEL_BUSCA_CUIDADOSA);
}

String CaiporaAuto::getSensorStatusJSON() {
    String json = "{";
    json += "\"LDR\": " + String(_ldr.readRaw()) + ", ";
    json += "\"Linha Esq\": " + String(_linhaEsq.leituraRaw()) + ", ";
    json += "\"Linha Dir\": " + String(_linhaDir.leituraRaw()) + ", ";
    json += "\"VL Frente Esq (mm)\": " + String(_vlFrenteEsq.leituraRaw()) + ", ";
    json += "\"VL Lat Esq (mm)\": " + String(_vlLateralEsq.leituraRaw()) + ", ";
    json += "\"VL Frente Dir (mm)\": " + String(_vlFrenteDir.leituraRaw()) + ", ";
    json += "\"VL Lat Dir (mm)\": " + String(_vlLateralDir.leituraRaw());
    json += "}";
    return json;
}