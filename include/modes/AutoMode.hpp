#pragma once
#include "ConfigServer.hpp"
#include "Drive.hpp"
#include "JS40F.hpp"
#include "QRE1113.hpp"
#include "MotionPlayer.hpp"
#include "RobotTypes.hpp"
#include "WeaponSystem.hpp"

class AutoMode {
  public:
    AutoMode();
    enum class SubState {
        SELECTING_ESTRATEGIA,
        DISCONNECTING_WIFI,
        READY,
        EXECUTING_ESTRATEGIA,
        HUNTING,
        ATTACKING
    };

    void init();
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
    // Sensors sensores;

    JS40F sensorEsq;
    JS40F sensorDir;
    JS40F sensorFrontal;

    QRE1113 sensorLinhaEsq;
    QRE1113 sensorLinhaDir;

    void executingEstrategia(Drive &motores);
    void buscaPadrao(Drive &motores);
    void ataquePadrao(Drive &motores);

    Direction _ultimoLado = Direction::left;
    bool _readyReceived = false;
    unsigned long _tempoDesligamento = 0;

    static constexpr int VEL_BUSCA_GIRO = 80;
    static constexpr int VEL_ATAQUE_MAX = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 50;
};