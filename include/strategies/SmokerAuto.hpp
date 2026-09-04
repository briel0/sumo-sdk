#pragma once
#include "CombatStrategy.hpp"
#include "JS40F.hpp"
#include "MotionPlayer.hpp"
#include "QRE1113.hpp"
#include "RobotTypes.hpp"

class SmokerAuto : public CombatStrategy {
  public:
    SmokerAuto();
    void init() override;
    void autoEngage(Drive &motores, WeaponSystem &armas) override;
    String getSensorStatusJSON() override;

  private:
    // Arco frontal: tres JS40F digitais.
    JS40F _sensorEsq;
    JS40F _sensorDir;
    JS40F _sensorFrontal;

    // Borda do dojo: dois QRE1113 analogicos, um por lado da frente.
    QRE1113 _linhaEsq;
    QRE1113 _linhaDir;

    // Toca as fugas de borda. E o unico dono dos motores enquanto estiver ativo.
    MotionPlayer _player;

    Direction _ultimoLado = Direction::left;

    static constexpr int VEL_BUSCA_GIRO = 80;
    static constexpr int VEL_ATAQUE_MAX = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 50;

    // Uma busca so, a mais simples: gira no proprio eixo ate cruzar com alguem.
    // As duas recebem o frame ja lido pelo autoEngage — nenhuma le sensor de novo.
    void _busca(Drive &motores, bool viuEsq, bool viuDir);
    void _ataque(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
};
