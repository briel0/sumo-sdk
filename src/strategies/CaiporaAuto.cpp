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

    _player.stop();
}

void CaiporaAuto::autoEngage(Drive &motores, WeaponSystem &armas) {
    armas.update();

    // Mesmo auto-armamento do RCMode: liga o servo assim que o robô entra em
    // ação (aqui: sempre, já que o auto anda pra frente o tempo todo) e deixa
    // o próprio WeaponSystem relaxar depois de RELAX_TIMEOUT_MS.
    if(!armas.isDeployed()) {
        armas.deploy();
    }

    // 1. Se o MotionPlayer estiver tocando alguma macro (ex: largada), ele assume o controle
    if(_player.isPlaying()) {
        _player.update(motores);
        return;
    }

    // 2. Auto simplificado: anda pra frente até o STOP do IR (botão 3, ver main.cpp).
    motores.setSpeed(VEL_ATAQUE, VEL_ATAQUE);
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