#pragma once
#include "CombatStrategy.hpp"
#include "JS40F.hpp"
#include "QRE1113.hpp"
#include "RobotTypes.hpp"

class SmokerAuto : public CombatStrategy {
  public:
    SmokerAuto();
    void init() override;
    void autoEngage(Drive &motores, WeaponSystem &armas) override;

  private:
    // Apenas declarando a existência deles.
    JS40F _sensorEsq;
    JS40F _sensorDir;
    JS40F _sensorFrontal;
    QRE1113 _linhaEsq;
    QRE1113 _linhaDir;

    Direction _ultimoLado = Direction::left;

    static constexpr int VEL_BUSCA_GIRO = 80;
    static constexpr int VEL_ATAQUE_MAX = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 50;

    void _busca(Drive &motores);
    void _ataque(Drive &motores);
};