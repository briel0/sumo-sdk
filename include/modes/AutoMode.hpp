#pragma once
#include "CombatStrategy.hpp"
#include "ConfigServer.hpp"
#include "Drive.hpp"
#include "JS40F.hpp"
#include "MotionPlayer.hpp"
#include "QRE1113.hpp"
#include "RobotTypes.hpp"
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
    ConfigServer configServer;
    AutoStrategy autoConfig;

    void executingEstrategia(Drive &motores);

    CombatStrategy *_estrategia = nullptr;
    HardwareCore   *_hardware   = nullptr;

    bool _readyReceived = false;
    unsigned int _tempoDesligamento = 0;
};