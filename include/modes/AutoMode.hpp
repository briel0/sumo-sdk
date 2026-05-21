#pragma once
#include "ConfigServer.hpp"
#include "Drive.hpp"
#include "MotionPlayer.hpp"
#include "RobotTypes.hpp"
#include "WeaponSystem.hpp"

// class Sensors;

class AutoMode {
  public:
    enum class SubState {
        SELECTING_SAQUE,
        ARMED_READY,
        EXECUTING_SAQUE,
        HUNTING,
        ATTACKING
    };

    enum class Saque {
        FRENTAO,
        CURVAO_ESQ,
        CURVAO_DIR,
        RECUADO
    };

    void init();
    void run(Drive &motores, WeaponSystem &armas, bool irArmed, bool irStart);

    SubState getSubState() const {
        return subState;
    }

  private:
    SubState subState = SubState::SELECTING_SAQUE;
    Saque saque = Saque::FRENTAO;
    MotionPlayer player;
    ConfigServer configServer;
    AutoStrategy autoConfig;
    // Sensors sensores;

    void handleArmedReady(bool irStart);
    void handleExecutingSaque(Drive &motores, WeaponSystem &armas);
    void handleCombat(Drive &motores, WeaponSystem &armas);
};