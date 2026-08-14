#pragma once
#include "CombatStrategy.hpp"
#include "RobotTypes.hpp"

class Drive;
class WeaponSystem;
class HardwareCore;

class SmokerAuto : public CombatStrategy {
  public:
    SmokerAuto() = default;
    void init(HardwareCore &hardware) override;
    void autoEngage(Drive &motores, WeaponSystem &armas, HardwareCore &hardware) override;

  private:
    // A estratégia não é mais dona dos sensores — ela consulta o HardwareCore.
    Direction _ultimoLado = Direction::left;

    static constexpr int VEL_BUSCA_GIRO      = 80;
    static constexpr int VEL_ATAQUE_MAX      = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 50;

    void _busca(Drive &motores, HardwareCore &hardware);
    void _ataque(Drive &motores, HardwareCore &hardware);
};
