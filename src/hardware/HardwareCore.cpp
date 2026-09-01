#include "HardwareCore.hpp"
#include "Config.hpp"

// A composição do HardwareCore é escolhida pela família de hardware
// (HW_FAMILY_*, definida em Config.hpp a partir do robô), não pelo nome do robô.
// Cada família só toca nos pinos/periféricos que realmente tem — por isso perfis
// de famílias mais simples nem precisam definir as constantes das outras.

#if defined(HW_FAMILY_FUMACINHA)
HardwareCore::HardwareCore()
    : _jsumoLeft(Config::PIN_JSUMO_ESQ, /*activeLow=*/false), _jsumoRight(Config::PIN_JSUMO_DIR, /*activeLow=*/false),
      _jsumoAsa(Config::PIN_JSUMO_ASA, /*activeLow=*/false), _irLeft(Config::PIN_IR_ESQ, /*activeLow=*/true),
      _irRight(Config::PIN_IR_DIR, /*activeLow=*/true),
      _lineSensorLeft(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD),
      _lineSensorRight(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD),
      _ldr(Config::PIN_LDR, Config::LDR_THRESHOLD, Config::LDR_FILTER_SIZE), _stealth(Config::PIN_STEALTH_EMITTER),
      _wing(Config::PIN_SERVO_ASA, Config::ASA_ANGLE_RETRACT, Config::ASA_ANGLE_LEFT, Config::ASA_ANGLE_RIGHT) {}
#elif defined(HW_FAMILY_CAIPORA)
// Endereços I2C distintos (0x30..0x33): os quatro VL53L0X nascem em 0x29 de
// fábrica, então cada um precisa ser acordado sozinho pelo XSHUT e reendereçado
// antes do próximo — ver a sequência em begin().
HardwareCore::HardwareCore()
    : _tofCentroEsq(Config::PIN_VL_CENT_ESQ, 0x30), _tofCentroDir(Config::PIN_VL_CENT_DIR, 0x31),
      _tofLateralEsq(Config::PIN_VL_ESQ, 0x32), _tofLateralDir(Config::PIN_VL_DIR, 0x33),
      _lineSensorLeft(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD),
      _lineSensorRight(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD),
      _ldr(Config::PIN_LDR, Config::LDR_THRESHOLD, Config::LDR_FILTER_SIZE) {}
#elif defined(HW_FAMILY_LEGACY)
HardwareCore::HardwareCore()
    : _front(Config::PIN_JS_FRONT, /*activeLow=*/true), _left(Config::PIN_JS_ESQ, /*activeLow=*/true),
      _right(Config::PIN_JS_DIR, /*activeLow=*/true), _lineSensorLeft(Config::PIN_LINHA_ESQ, Config::LINHA_THRESHOLD),
      _lineSensorRight(Config::PIN_LINHA_DIR, Config::LINHA_THRESHOLD), _stealth(Config::PIN_STEALTH_EMITTER),
      _wing(Config::PIN_SERVO_ASA, Config::ASA_ANGLE_RETRACT, Config::ASA_ANGLE_LEFT, Config::ASA_ANGLE_RIGHT) {}
#else // HW_FAMILY_NONE
HardwareCore::HardwareCore() {}
#endif

void HardwareCore::begin() {
    if(_initialized) {
        return;
    }

#if defined(HW_FAMILY_FUMACINHA)
    _jsumoLeft.init();
    _jsumoRight.init();
    _jsumoAsa.init();
    _irLeft.init();
    _irRight.init();
    _lineSensorLeft.init();
    _lineSensorRight.init();
    _ldr.init();
    _stealth.init();
    _wing.init();
    Serial.println("[HW] Núcleo de Hardware inicializado (família FUMACINHA).");
#elif defined(HW_FAMILY_CAIPORA)
    Wire.begin();

    // Todos os VL53L0X saem de fábrica no MESMO endereço (0x29). Se dois
    // acordarem juntos, os dois respondem e o barramento vira lixo. Por isso:
    // apaga os quatro, e só então liga e reendereça um a um — cada init() sobe o
    // XSHUT do seu sensor e grava o endereço definitivo antes do próximo subir.
    _tofCentroEsq.disable();
    _tofCentroDir.disable();
    _tofLateralEsq.disable();
    _tofLateralDir.disable();
    delay(20);

    _tofCentroEsq.init();
    _tofCentroDir.init();
    _tofLateralEsq.init();
    _tofLateralDir.init();

    _lineSensorLeft.init();
    _lineSensorRight.init();
    _ldr.init();
    Serial.println("[HW] Núcleo de Hardware inicializado (família CAIPORA).");
#elif defined(HW_FAMILY_LEGACY)
    _front.init();
    _left.init();
    _right.init();
    _lineSensorLeft.init();
    _lineSensorRight.init();
    _stealth.init();
    _wing.init();
    Serial.println("[HW] Núcleo de Hardware inicializado (família LEGACY).");
#else // HW_FAMILY_NONE
    Serial.println("[HW] Sem front-end de sensores (família NONE) — nada a inicializar.");
#endif

    _initialized = true;
}

void HardwareCore::update() {
#if defined(HW_FAMILY_FUMACINHA)
    // --- Leitura + filtragem dos sensores ---
    // O JSumo lateral só é confiável com o emissor ligado (mesmo transistor que
    // alimenta o CI inversor) — o IR puro fica sempre energizado e cobre a
    // detecção lateral enquanto o modo furtivo mantém o emissor desligado.
    bool emittersOn = _stealth.isEnabled();
    _leftDetected   = emittersOn ? _jsumoLeft.temAlvo() : _irLeft.temAlvo();
    _rightDetected  = emittersOn ? _jsumoRight.temAlvo() : _irRight.temAlvo();
    _frontDetected  = _jsumoAsa.temAlvo(); // emissor da asa nunca é desligado
    _lineLeft       = _lineSensorLeft.temLinhaBranca();
    _lineRight      = _lineSensorRight.temLinhaBranca();

    _ldr.update();
    _ldrFiltered    = _ldr.filtered();
    _opponentOnRamp = _ldr.belowThreshold();

    // --- Atuação dos periféricos de estado ---
    _stealth.set(_stealthIntent);
    if(_wingIntent != _wing.position()) {
        _wing.moveTo(_wingIntent);
    }
#elif defined(HW_FAMILY_CAIPORA)
    // --- Leitura + filtragem dos sensores ---
    // Laterais extremos = detecção lateral (papel dos JSumo no Fumacinha).
    _leftDetected  = _tofLateralEsq.temOponente(Config::TOF_THRESHOLD_MM);
    _rightDetected = _tofLateralDir.temOponente(Config::TOF_THRESHOLD_MM);

    // Os DOIS centrais juntos = alvo realmente centrado à frente. É o mesmo
    // critério que o FuegoAuto usa com os laterais dele (robô sem sensor
    // frontal), só que aplicado ao par central, que é mais seletivo.
    _frontDetected = _tofCentroEsq.temOponente(Config::TOF_THRESHOLD_MM) &&
                     _tofCentroDir.temOponente(Config::TOF_THRESHOLD_MM);

    _lineLeft  = _lineSensorLeft.temLinhaBranca();
    _lineRight = _lineSensorRight.temLinhaBranca();

    _ldr.update();
    _ldrFiltered    = _ldr.filtered();
    _opponentOnRamp = _ldr.belowThreshold();

    // Sem periféricos de estado nesta família: as intenções de _stealthIntent e
    // _wingIntent ficam guardadas e simplesmente não são aplicadas em lugar nenhum.
#elif defined(HW_FAMILY_LEGACY)
    // --- Leitura + filtragem dos sensores (sem LDR de rampa nesta família) ---
    _frontDetected = _front.temAlvo();
    _leftDetected  = _left.temAlvo();
    _rightDetected = _right.temAlvo();
    _lineLeft      = _lineSensorLeft.temLinhaBranca();
    _lineRight     = _lineSensorRight.temLinhaBranca();

    // --- Atuação dos periféricos de estado ---
    _stealth.set(_stealthIntent);
    if(_wingIntent != _wing.position()) {
        _wing.moveTo(_wingIntent);
    }
#else  // HW_FAMILY_NONE
    // Sem sensores nem atuadores de combate — nada a varrer ou aplicar.
#endif
}

// --- Leituras cruas para o diagnóstico de bancada ---------------------------
// Definidas por família: cada uma só devolve valor real onde o sensor existe.

#if defined(HW_FAMILY_FUMACINHA)
bool HardwareCore::jsumoLeftRaw() const {
    return _jsumoLeft.temAlvo();
}
bool HardwareCore::jsumoRightRaw() const {
    return _jsumoRight.temAlvo();
}
bool HardwareCore::jsumoAsaRaw() const {
    return _jsumoAsa.temAlvo();
}
bool HardwareCore::irLeftRaw() const {
    return _irLeft.temAlvo();
}
bool HardwareCore::irRightRaw() const {
    return _irRight.temAlvo();
}
#else
bool HardwareCore::jsumoLeftRaw() const {
    return false;
}
bool HardwareCore::jsumoRightRaw() const {
    return false;
}
bool HardwareCore::jsumoAsaRaw() const {
    return false;
}
bool HardwareCore::irLeftRaw() const {
    return false;
}
bool HardwareCore::irRightRaw() const {
    return false;
}
#endif

#if defined(HW_FAMILY_NONE)
uint16_t HardwareCore::lineLeftRaw() const {
    return 0;
}
uint16_t HardwareCore::lineRightRaw() const {
    return 0;
}
#else
uint16_t HardwareCore::lineLeftRaw() const {
    return _lineSensorLeft.leituraRaw();
}
uint16_t HardwareCore::lineRightRaw() const {
    return _lineSensorRight.leituraRaw();
}
#endif
