#pragma once
#include "ConfigServer.hpp"
#include "DiagnosticsPanel.hpp"
#include "Drive.hpp"
#include "Fumacinha_FSM.hpp"
#include "MotionPlayer.hpp"
#include "RobotTypes.hpp"
#include "WeaponSystem.hpp"

class HardwareCore;

/**
    @class FumacinhaMode
    @brief Orquestra o modo AUTO para robôs que usam a Fumacinha_FSM em vez do
           par MotionPlayer + CombatStrategy do AutoMode legado.

    Reaproveita o mesmo ConfigServer (WiFi AP + HUD + captive portal) e o mesmo
    protocolo de seleção (`/set-strat`) do AutoMode, só trocando AutoStrategy por
    CombatProfile como payload.

    A diferença estrutural real está depois da largada: no AutoMode legado,
    "estratégia inicial" (macro cega via MotionPlayer) e "modo iterativo"
    (CombatStrategy::autoEngage a cada frame) são dois sub-estados distintos,
    porque são dois mecanismos diferentes. Aqui não — OPENING, SEARCHING e
    ATTACKING já são só fases internas da MESMA Fumacinha_FSM, então a partir da
    largada este modo só entrega o frame para combatFsm.update() e ela mesma
    decide quando trocar de fase. Por isso SubState não tem um equivalente a
    EXECUTING_ESTRATEGIA: FIGHTING cobre as três fases de uma vez.

    Também hospeda os toggles de diagnóstico de bancada (teste de sensores /
    teste de motores) ligados pela mesma HUD onde as táticas são selecionadas
    (ConfigServer::isSensorTestActive()/isMotorTestActive()). Rodam como um
    overlay por cima de SELECTING_ESTRATEGIA/DISCONNECTING_WIFI/READY — não
    bloqueiam a seleção/transmissão de tática, que continua funcionando
    normalmente enquanto um teste está ativo. Nunca rodam durante FIGHTING:
    a Fumacinha_FSM assume o frame inteiro assim que a luta começa.
*/
class FumacinhaMode {
  public:
    FumacinhaMode() = default;

    enum class SubState {
        SELECTING_ESTRATEGIA,
        DISCONNECTING_WIFI,
        READY,
        FIGHTING, // delega inteiramente para Fumacinha_FSM::update()
    };

    /**
    @brief Inicializa o HardwareCore (dependência obrigatória — sensores e
           periféricos de estado, sem gate de Config::USES_HARDWARE_CORE como o
           AutoMode legado faz, já que a Fumacinha_FSM não funciona sem eles) e
           sobe o ConfigServer para a seleção tática.
    */
    void init(HardwareCore &hardware);
    void run(Drive &motores, WeaponSystem &armas, bool irStart, bool irReady, StatusLed &statusLed);

    SubState getSubState() const {
        return subState;
    }

    bool readyReceived() const {
        return _readyReceived;
    }

  private:
    SubState subState = SubState::SELECTING_ESTRATEGIA;
    ConfigServer configServer;
    Fumacinha_FSM combatFsm;
    CombatProfile combatProfile;
    HardwareCore *_hardware = nullptr;

    bool _readyReceived = false;
    unsigned int _tempoDesligamento = 0;

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

    void _executeCombat(Drive &motores);
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
    // pela Fumacinha_FSM em combate — não um attach() paralelo/isolado), pra
    // esse teste realmente validar o caminho real, não só o hardware sozinho.
    int _servoTestStep = 0;
    unsigned long _servoTestStartMs = 0;

    static constexpr unsigned long SERVO_TEST_STEP_MS = 1500; // tempo pra ver o movimento completar

    void runServoTestCycle(StatusLed &statusLed);
};
