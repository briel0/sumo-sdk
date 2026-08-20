#pragma once
#include "CombatStrategy.hpp"
#include "RobotTypes.hpp"
#include "ToFSensor.hpp"
#include "QRE1113.hpp"
#include "LDR.hpp"
#include "MotionPlayer.hpp"

class CaiporaAuto : public CombatStrategy {
  public:
    // Pinos (valores aleatórios, altere depois)
    static constexpr int PIN_LDR = 4;
    static constexpr int PIN_XSHUT_FRENTE_ESQ = 5;
    static constexpr int PIN_XSHUT_LATERAL_ESQ = 12;
    static constexpr int PIN_XSHUT_FRENTE_DIR = 13;
    static constexpr int PIN_XSHUT_LATERAL_DIR = 14;
    static constexpr int PIN_LINHA_ESQ = 26;
    static constexpr int PIN_LINHA_DIR = 27;

    CaiporaAuto();
    void init() override;
    void autoEngage(Drive &motores, WeaponSystem &armas) override;
    String getSensorStatusJSON() override;

  private:
    LDR _ldr;
    ToFSensor _vlFrenteEsq;
    ToFSensor _vlLateralEsq;
    ToFSensor _vlFrenteDir;
    ToFSensor _vlLateralDir;
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