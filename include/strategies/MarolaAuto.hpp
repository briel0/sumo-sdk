#pragma once
#include "CombatStrategy.hpp"
#include "MotionPlayer.hpp"
#include "RobotTypes.hpp"
#include "hardware/sensors/LDR.hpp"
#include "hardware/sensors/QRE1113.hpp"

class MarolaAuto : public CombatStrategy {
  public:
    MarolaAuto();
    void init() override;
    void autoEngage(Drive &motores, WeaponSystem &armas) override;
    String getSensorStatusJSON() override;

  private:
    LDR _ldrEsq;
    LDR _ldrDir;
    LDR _ldrFront;
    QRE1113 _linhaEsq;
    QRE1113 _linhaDir;
    MotionPlayer _player;

    // Auto simplificado: arma o servo e anda pra frente até o STOP do IR
    // (botão 3, ver main.cpp) — sem lógica de busca/ataque. LDRs e QRE1113
    // continuam lidos só pro getSensorStatusJSON().
    static constexpr int VEL_ATAQUE_MAX = 100;
};
