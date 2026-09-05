#include "CaiporaAuto.hpp"
#include "Config.hpp"
#include "Drive.hpp"
#include "WeaponSystem.hpp"

CaiporaAuto::CaiporaAuto()
    : _ldr(PIN_LDR), _vlFrenteEsq(PIN_XSHUT_FRENTE_ESQ, 0x30), _vlLateralEsq(PIN_XSHUT_LATERAL_ESQ, 0x31),
      _vlFrenteDir(PIN_XSHUT_FRENTE_DIR, 0x32), _vlLateralDir(PIN_XSHUT_LATERAL_DIR, 0x33) {}

void CaiporaAuto::init() {
    _ldr.init();

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
    // 1. Se o MotionPlayer estiver tocando alguma macro (ex: largada), ele assume o controle
    if(_player.isPlaying()) {
        _player.update(motores);
        return;
    }

    // 2. VL laterais: leitura espaçada (ver INTERVALO_TOF_MS) — sem isso os dois
    // reads bloqueantes travariam o loop.
    unsigned long agora = millis();
    if(agora - _ultimaLeituraToF >= INTERVALO_TOF_MS) {
        _ultimaLeituraToF = agora;
        _viuLateralEsq = _vlLateralEsq.temOponente(LIMIAR_BUSCA_MM);
        _viuLateralDir = _vlLateralDir.temOponente(LIMIAR_BUSCA_MM);
    }

    // 3. LDR faz o papel do JS40F frontal do Arruela: abaixo do limiar, algo
    // bloqueou a luz de cima — alvo na cara.
    bool viuFrente = _ldr.readRaw() < LIMIAR_LDR;

    _busca(motores, _viuLateralEsq, _viuLateralDir, viuFrente);
}

void CaiporaAuto::_busca(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente) {
    // Alvo na cara (LDR): vai com tudo.
    if(viuFrente) {
        motores.setSpeed(VEL_ATAQUE, VEL_ATAQUE);
        return;
    }
    // Só a esquerda ve: gira pra esquerda devagarinho.
    if(viuEsq) {
        _ultimoLado = Direction::left;
        motores.setSpeed(-VEL_BUSCA_CUIDADOSA, VEL_BUSCA_CUIDADOSA);
        return;
    }
    // Só a direita ve: gira pra direita devagarinho.
    if(viuDir) {
        _ultimoLado = Direction::right;
        motores.setSpeed(VEL_BUSCA_CUIDADOSA, -VEL_BUSCA_CUIDADOSA);
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
    json += "\"VL Frente Esq (mm)\": " + String(_vlFrenteEsq.leituraRaw()) + ", ";
    json += "\"VL Lat Esq (mm)\": " + String(_vlLateralEsq.leituraRaw()) + ", ";
    json += "\"VL Frente Dir (mm)\": " + String(_vlFrenteDir.leituraRaw()) + ", ";
    json += "\"VL Lat Dir (mm)\": " + String(_vlLateralDir.leituraRaw());
    json += "}";
    return json;
}