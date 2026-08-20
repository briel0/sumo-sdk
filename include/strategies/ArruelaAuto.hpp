#pragma once
#include "CombatStrategy.hpp"
#include "JS40F.hpp"
#include "RobotTypes.hpp"
#include "ToFSensor.hpp"

class ArruelaAuto : public CombatStrategy {
  public:
    ArruelaAuto();
    void init() override;
    void autoEngage(Drive &motores, WeaponSystem &armas) override;
    String getSensorStatusJSON() override;

  private:
    // Apenas declarando a existência deles.
    JS40F _sensorEsq;
    JS40F _sensorDir;
    JS40F _sensorFrontal;
    ToFSensor _sensorDistancia;

    Direction _ultimoLado = Direction::left;

    static constexpr int VEL_BUSCA_GIRO = 90;
    static constexpr int VEL_ATAQUE_MAX = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 60;

    void _busca(Drive &motores, bool viuEsq, bool viuDir);
    void _ataque(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
};