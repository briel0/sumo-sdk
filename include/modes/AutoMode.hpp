#pragma once
#include "ConfigServer.hpp"
#include "Drive.hpp"
#include "JS40F.hpp"
#include "MotionPlayer.hpp"
#include "RobotTypes.hpp"
#include "WeaponSystem.hpp"

class AutoMode {
  public:
    AutoMode();
    enum class SubState {
        SELECTING_ESTRATEGIA,
        READY,
        EXECUTING_ESTRATEGIA,
        HUNTING,
        ATTACKING
    };

    void init();
    void run(Drive &motores, WeaponSystem &armas, bool irStart);

    SubState getSubState() const {
        return subState;
    }

  private:
    SubState subState = SubState::SELECTING_ESTRATEGIA;
    MotionPlayer estrategiaPlayer;
    ConfigServer configServer;
    AutoStrategy autoConfig;
    // Sensors sensores;

    JS40F sensorEsq;
    JS40F sensorDir;
    JS40F sensorFrontal;

    void executingEstrategia(Drive &motores);
    void buscaPadrao(Drive &motores);
    void ataquePadrao(Drive &motores);

    Direction _ultimoLado = Direction::left;

    static constexpr int VEL_BUSCA_GIRO = 60;
    static constexpr int VEL_ATAQUE_MAX = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 50;
};