#pragma once
#include "RobotTypes.hpp"
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

class ConfigServer {
  public:
    ConfigServer(uint16_t port = 80);
    void begin();
    void shutdown();
    void update();

    void teardownBluetooth();
    bool setupAccessPoint();
    void setupWebRoutes();

    bool consumePayload(AutoStrategy &outStrategy);

    /**
    @brief Mesmo papel de consumePayload(), para robôs que usam FumacinhaAuto
           (CombatProfile) em vez do par MotionPlayer + CombatStrategy legado.
    */
    bool consumeCombatProfile(CombatProfile &outProfile);

    /**
    @brief Estado ao vivo (não "consumido" — lido a cada frame) dos toggles de
           diagnóstico de bancada, ligados via /set-test na mesma HUD onde as
           táticas são selecionadas. Mutuamente exclusivos entre si: ligar um
           desliga os outros dois, já que compartilham os mesmos 5 LEDs e (no
           caso de motor/servo) os próprios atuadores.
    */
    bool isSensorTestActive() const {
        return sensorTestActive;
    }
    bool isMotorTestActive() const {
        return motorTestActive;
    }
    bool isServoTestActive() const {
        return servoTestActive;
    }

    /**
    @brief Força os três toggles de teste de volta a "desligado". Chamado ao
           entrar em combate de verdade — testes de bancada não têm permissão
           de rodar durante a luta.
    */
    void clearTests();

    /**
    @brief Publica o snapshot mais recente das leituras de bancada (sensores ou
           motores, conforme o teste ativo) como JSON pronto pra servir em
           /get-test-data. Quem monta o JSON é o dono do HardwareCore/Drive
           (FumacinhaMode) — o ConfigServer só guarda e serve a string, sem
           conhecer o esquema dos campos.
    */
    void setTestReadout(const String &json) {
        testReadoutJson = json;
    }

    /**
    @brief Testador de macro ao vivo (rota /test-macro). O construtor de passos
           do dashboard manda uma sequência (vel. esq / vel. dir / tempo por
           passo) pra ser disparada NA HORA, durante a configuração — só pra
           calibrar movimento, sem fazer parte da luta. Padrão polled (igual
           consumeCombatProfile): a rota só guarda os passos + marca pendente; é
           o dono do Drive (FumacinhaMode) que consome e toca via MotionPlayer,
           evitando acionar motor de dentro da task async do HTTP.
    @param dst      buffer do chamador pra onde os passos são copiados.
    @param dstCap   capacidade de dst.
    @param outCount nº de passos copiados.
    @return true se havia um macro novo pendente.
    */
    bool consumeMacroTest(MotionStep *dst, int dstCap, int &outCount);

  private:
    AsyncWebServer server;
    AutoStrategy   currentAutoStrategy;
    CombatProfile  currentCombatProfile;
    DNSServer      dnsServer;

    bool sensorTestActive = false;
    bool motorTestActive  = false;
    bool servoTestActive  = false;
    String testReadoutJson = "{}";

    // Buffer do macro de teste recebido em /test-macro (máx. 8 passos, igual ao
    // construtor do dashboard). _macroTestPending sinaliza que chegou um novo.
    static constexpr int      MAX_MACRO_STEPS = 8;
    MotionStep                testMacroSteps[MAX_MACRO_STEPS];
    int                       testMacroCount   = 0;
    bool                      testMacroPending = false;
};