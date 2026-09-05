#pragma once
#include "RobotTypes.hpp"
#include <Arduino.h> // String

// Substituto do ConfigServer (HTTP/WiFi) por BLE nativo — mesma interface
// pública que o AutoMode.cpp já usa (consumePayload/consumeCombatProfile/
// isSensorTestActive/isMotorTestActive/isServoTestActive/clearTests/
// setTestReadout/consumeMacroTest/begin/update/shutdown), pra trocar um pelo
// outro sem tocar em AutoMode nem em nenhuma estratégia de combate.
//
// Motivo da troca: BLE nativo (Bluedroid, BLEDevice.h) e Bluepad32 (BTstack)
// não cabem no mesmo binário nesse fork da Espressif — mesma trava resolvida
// na feat/bluetooth-config. Por isso o RC saiu desta branch (ver os envs
// fuego/fumacinha no platformio.ini, que limpam o platform_packages do
// Bluepad32 e caem no framework Arduino-ESP32 padrão).
//
// Quem escolhe entre este e o ConfigServer é o Config.hpp, por robô, via o
// alias ActiveConfigServer — AutoMode usa esse alias, não um tipo fixo, já
// que a classe é compartilhada por robôs que ainda precisam do ConfigServer
// (WiFi) e robôs que já migraram pra este.
//
// Transporte: uma ÚNICA characteristic BLE (0000FF10/0000FF11, mesmos UUIDs
// da feat/bluetooth-config), com um protocolo pequeno por cima — 1 byte de
// comando no WRITE, resposta correspondente já deixada pronta pro READ
// seguinte. Ver .cpp pra tabela de comandos e o layout binário do
// CombatProfile (a HUD do Fumacinha nunca manda os campos de CombatTuning —
// só os 9 campos por-luta; tuning fica sempre no default do struct, igual o
// comportamento atual via HTTP).
class BleConfigServer {
  public:
    BleConfigServer() = default;

    void begin();
    void shutdown();
    void update();

    bool consumePayload(AutoStrategy &outStrategy);

    /**
    @brief Mesmo papel de consumePayload(), para robôs que usam FumacinhaAuto
           (CombatProfile) em vez do par MotionPlayer + CombatStrategy legado.
    */
    bool consumeCombatProfile(CombatProfile &outProfile);

    bool isSensorTestActive() const {
        return _sensorTestActive;
    }
    bool isMotorTestActive() const {
        return _motorTestActive;
    }
    bool isServoTestActive() const {
        return _servoTestActive;
    }

    /**
    @brief Força os três toggles de teste de volta a "desligado". Chamado ao
           entrar em combate de verdade.
    */
    void clearTests();

    /**
    @brief Publica o snapshot mais recente das leituras de bancada como JSON
           pronto pra servir no próximo GET_SENSORS. Definida no .cpp (não
           inline): protege a escrita com a mesma seção crítica que a leitura
           em handleWrite() usa — a leitura roda na task do Bluedroid,
           diferente da task do loop() que chama isso, e String não é
           thread-safe (ver o bug de "sensor piscando" resolvido na
           feat/bluetooth-config com o mesmo problema).
    */
    void setTestReadout(const String &json);

    /**
    @brief Testador de macro ao vivo. Mesmo contrato do ConfigServer::
           consumeMacroTest — copia até dstCap passos pro buffer do chamador.
    */
    bool consumeMacroTest(MotionStep *dst, int dstCap, int &outCount);

    // Chamado pelo callback de WRITE da characteristic (classe interna do
    // .cpp) — público porque quem chama não é membro de BleConfigServer, é
    // um BLECharacteristicCallbacks separado que guarda um ponteiro pra essa
    // instância. Ver BleConfigServerCallbacks no .cpp.
    void handleWrite(class BLECharacteristic *characteristic);

  private:
    class BLECharacteristic *_characteristic = nullptr;

    AutoStrategy  _currentAutoStrategy;
    CombatProfile _currentCombatProfile;

    bool _sensorTestActive = false;
    bool _motorTestActive = false;
    bool _servoTestActive = false;
    String _testReadoutJson = "{}";

    static constexpr int MAX_MACRO_STEPS = 8;
    MotionStep _testMacroSteps[MAX_MACRO_STEPS];
    int _testMacroCount = 0;
    bool _testMacroPending = false;
};
