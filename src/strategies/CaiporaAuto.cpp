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

    _busca(motores, _rawFrenteEsq, _rawLateralEsq, _rawFrenteDir, _rawLateralDir, rawLdr);
}

void CaiporaAuto::_busca(Drive &motores, uint16_t rawFrenteEsq, uint16_t rawLateralEsq, uint16_t rawFrenteDir,
                         uint16_t rawLateralDir, int rawLdr) {
    
    if(rawLdr < LIMIAR_LDR) {
        motores.setSpeed(VEL_ATAQUE, VEL_ATAQUE);
        return;
    }
    
    if(rawFrenteEsq <= 500 && rawFrenteDir <= 500){
        motores.setSpeed(50, 50);
        return;
    }

    if(rawLateralEsq <= 500){
        motores.setSpeed(-55, 55);
        return;
    }

    if(rawLateralDir <= 500){
        motores.setSpeed(55, -55);
    }

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
