#pragma once
#include "CombatStrategy.hpp"
#include "ConfigServer.hpp"
#include "DiagnosticsPanel.hpp" // MotorTestState + painel de LED dos testes de bancada
#include "Drive.hpp"
#include "JS40F.hpp"
#include "MotionPlayer.hpp"
#include "QRE1113.hpp"
#include "RobotTypes.hpp"
#include "StatusLED.hpp"
#include "WeaponSystem.hpp"

class HardwareCore;

class AutoMode {
  public:
    AutoMode() = default;
    enum class SubState {
        SELECTING_ESTRATEGIA,
        DISCONNECTING_WIFI,
        READY,
        EXECUTING_ESTRATEGIA,
        FIGHTING,
    };

    void init(CombatStrategy &estrategia, HardwareCore &hardware);
    // StatusLed entra como parâmetro porque o AutoMode passou a ser o dono
    // ÚNICO do feedback de LED no modo AUTO — inclusive o dos testes de
    // bancada. Antes isso vivia espalhado no loop() do main.cpp, e quando o
    // painel de teste e o strategyWave escreviam no mesmo frame os LEDs
    // piscavam.
    void run(Drive &motores, WeaponSystem &armas, bool irStart, bool irReady, StatusLed &statusLed);

    SubState getSubState() const {
        return subState;
    }

    bool readyReceived() const {
        return _readyReceived;
    }

  private:
    SubState subState = SubState::SELECTING_ESTRATEGIA;
    MotionPlayer estrategiaPlayer;
    ConfigServer configServer;
    AutoStrategy autoConfig;

    // Payload da HUD do Fumacinha. Só é consumido quando
    // Config::USES_FUMACINHA_FSM está ligado — nos demais robôs a HUD
    // legada manda um AutoStrategy e este campo fica no default.
    CombatProfile combatProfile;

    // Latch de LED: vira true assim que qualquer teste de bancada roda nesta
    // sessão AUTO. Enquanto false e sem teste ativo, os LEDs fazem a onda de
    // "esperando o site" (strategyWave). Depois que um teste é usado e desligado,
    // os LEDs ficam verdes em vez de voltar pra onda (pedido explícito). É
    // ignorado na fase READY, onde o feedback de largada sempre tem prioridade.
    bool _benchTested = false;

    // --- Testador de macro ao vivo (fora de combate) ----------------------
    // Sequência disparada pelo construtor de passos do dashboard (/test-macro),
    // tocada pelo MotionPlayer durante a configuração só pra calibrar movimento.
    // Buffer próprio (não aponta pro ConfigServer) — o player guarda só o
    // ponteiro, então os passos precisam viver aqui enquanto tocam.
    MotionPlayer _macroPlayer;
    MotionStep _macroSteps[8];

    // --- Diagnóstico de bancada (overlay fora de combate) ------------------
    MotorTestState _motorTestState = MotorTestState::PARADO;
    unsigned long _motorTestStartMs = 0;

    static constexpr unsigned long MOTOR_TEST_STEP_MS = 1000;
    static constexpr int MOTOR_TEST_PWM = 60; // potência moderada — teste de bancada, não combate

    void runSensorTestCycle(Drive &motores, StatusLed &statusLed);
    void runMotorTestCycle(Drive &motores, StatusLed &statusLed);
    void applyMotorTestState(Drive &motores);

    /**
    @brief Traduz um MotorTestState pro par de PWM (esquerda, direita) que ele
           aplica. Único ponto de verdade — usado tanto pra acionar os motores
           quanto pra montar o log de bancada, evitando duplicar o switch.
    */
    void motorPwmForState(MotorTestState state, int &leftPWM, int &rightPWM) const;
    static const char *motorTestStateLabel(MotorTestState state);

    // --- Teste de servo (asa) — mesmo padrão do teste de motor, mas comandando
    // o servo via HardwareCore::setWing() (o MESMO caminho de produção usado
    // pela FumacinhaAuto em combate — não um attach() paralelo/isolado), pra
    // esse teste realmente validar o caminho real, não só o hardware sozinho.
    int _servoTestStep = 0;
    unsigned long _servoTestStartMs = 0;

    static constexpr unsigned long SERVO_TEST_STEP_MS = 1500; // tempo pra ver o movimento completar

    void runServoTestCycle(StatusLed &statusLed);


    void executingEstrategia(Drive &motores);

    CombatStrategy *_estrategia = nullptr;
    HardwareCore   *_hardware   = nullptr;

    bool _readyReceived = false;
    unsigned int _tempoDesligamento = 0;
};