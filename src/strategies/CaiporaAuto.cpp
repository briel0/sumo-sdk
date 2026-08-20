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
}

void CaiporaAuto::_busca(Drive &motores, bool viuLateralEsq, bool viuLateralDir) {
    if(viuLateralEsq) {
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
        return;
    }
    if(viuLateralDir) {
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
        return;
    }
    if(_ultimoLado == Direction::right)
        motores.setSpeed(VEL_BUSCA_GIRO, -VEL_BUSCA_GIRO);
    else
        motores.setSpeed(-VEL_BUSCA_GIRO, VEL_BUSCA_GIRO);
}

void CaiporaAuto::_ataque(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {
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