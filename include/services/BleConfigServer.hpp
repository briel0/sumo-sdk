#pragma once
#include "RobotTypes.hpp"
#include <Arduino.h> // String — RobotTypes.hpp não traz Arduino.h, precisa explícito
#include <functional>

// Substituto do ConfigServer (HTTP/WiFi) por BLE — mesma interface pública
// usada pelo AutoMode.cpp, pra trocar um pelo outro sem tocar em AutoMode.
//
// Transporte: uma ÚNICA characteristic BLE (mesmo serviço/characteristic de
// antes: 0000FF10/0000FF11), com um protocolo pequeno por cima — 1 byte de
// comando no WRITE, resposta correspondente já deixada pronta pro READ
// seguinte. Ver .cpp pra tabela de comandos.
//
// Stack: lib BLE nativa da Espressif (Bluedroid, BLEDevice.h/BLEServer.h),
// não mais BTstack via Bluepad32. Motivo da troca: com o GATT rodando em
// cima do MESMO BTstack que o Bluepad32 usa pra escanear gamepads, o
// advertising nunca saía no ar de verdade — todos os comandos HCI
// (LE Set Advertising Parameters/Data/Enable) voltavam status 0x00 (OK), mas
// o robô não aparecia em NENHUM scanner (nRF Connect incluído), nem forçando
// reenviar o enable a cada 1s. Suspeita: coexistência de rádio com o
// scan contínuo do Bluepad32 por controles. Pra resolver de vez, RC e AUTO
// viraram firmwares separados (ver tools/RCFirmware/) e o AUTO não linka
// mais Bluepad32/BTstack — esse arquivo usa o stack BLE "normal" do ESP32,
// sem ninguém mais disputando o rádio.
class BleConfigServer {
  public:
    BleConfigServer() = default;

    void begin();
    void shutdown();
    void update();

    bool consumePayload(AutoStrategy &outStrategy);

    // Definido no .cpp (não inline): protege a escrita com a mesma seção
    // crítica que a leitura em handleWrite() usa — a leitura roda na task
    // do Bluedroid, diferente da task do loop() que chama isso, e String
    // não é thread-safe.
    void setTestReadout(const String &json);

    void setMacroTestCallback(std::function<void(MotionSequence)> cb) {
        _macroTestCallback = cb;
    }

    void setMotorTestCallback(std::function<void(bool)> cb) {
        _motorTestCallback = cb;
    }

    void setSensorTestCallback(std::function<void(bool)> cb) {
        _sensorTestCallback = cb;
    }

    // Chamado pelo callback de WRITE da characteristic (classe interna do
    // .cpp) — público porque quem chama não é membro de BleConfigServer,
    // é um BLECharacteristicCallbacks separado que guarda um ponteiro pra
    // essa instância. Ver BleConfigServerCallbacks no .cpp.
    void handleWrite(class BLECharacteristic *characteristic);

  private:
    class BLECharacteristic *_characteristic = nullptr;

    std::function<void(MotionSequence)> _macroTestCallback = nullptr;
    std::function<void(bool)> _motorTestCallback = nullptr;
    std::function<void(bool)> _sensorTestCallback = nullptr;

    String _testReadoutJson = "{}";
    AutoStrategy _currentAutoStrategy;
    MotionStep _macroTestSteps[8];
};
