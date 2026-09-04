#pragma once
#include "CombatStrategy.hpp"
#include "RobotTypes.hpp"
#include "hardware/sensors/LDR.hpp"
#include "hardware/sensors/QRE1113.hpp"
#include "MotionPlayer.hpp"

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

    Direction _ultimoLado = Direction::left;

    static constexpr int VEL_BUSCA_GIRO = 90;
    static constexpr int VEL_ATAQUE_MAX = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 60;

    void _busca(Drive &motores, bool viuEsq, bool viuDir);
    void _ataque(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
};
