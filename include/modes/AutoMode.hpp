#pragma once
#include "BleConfigServer.hpp"
#include "CombatStrategy.hpp"
#include "Drive.hpp"
#include "JS40F.hpp"
#include "MotionPlayer.hpp"
#include "QRE1113.hpp"
#include "RobotTypes.hpp"
#include "WeaponSystem.hpp"

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

    void init(CombatStrategy &estrategia);
    void run(Drive &motores, WeaponSystem &armas, bool irStart, bool irReady);

    SubState getSubState() const {
        return subState;
    }

    bool readyReceived() const {
        return _readyReceived;
    }

  private:
    SubState subState = SubState::SELECTING_ESTRATEGIA;
    MotionPlayer estrategiaPlayer;
    BleConfigServer configServer;
    AutoStrategy autoConfig;

    void executingEstrategia(Drive &motores);

    CombatStrategy *_estrategia = nullptr;

    bool _readyReceived = false;
    bool _testingMacro = false;
    bool _testingMotor = false;
    bool _testingSensor = false;
    bool _startMacroTest = false;
    bool _weaponCommandPending = false;
    bool _weaponCommandArm = false;
    MotionSequence _macroToTest;
    unsigned int _tempoDesligamento = 0;
    unsigned long _ultimoReadout = 0;
};