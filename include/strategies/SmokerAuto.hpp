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
    String getSensorStatusJSON() override;

  private:
    // Arco frontal: tres JS40F digitais.
    JS40F _sensorEsq;
    JS40F _sensorDir;
    JS40F _sensorFrontal;

    // Borda do dojo: continuam lidos so pro getSensorStatusJSON(), sem fuga
    // de linha.
    QRE1113 _linhaEsq;
    QRE1113 _linhaDir;

    Direction _ultimoLado = Direction::left;

    // Arma ao entrar em combate e decide busca/ataque frame a frame, igual a
    // BUSCA PADRAO do ArruelaAuto (mesma logica, so troca ToF/JS40F esq-dir
    // pelos 3 JS40F). ATAQUE: frontal viu alguem, avanca no talo sem girar.
    // BUSCA: lateral viu alguem, gira pra la; cego nos tres, gira pro ultimo
    // lado que viu alguem.
    static constexpr int VEL_BUSCA_GIRO = 80;
    static constexpr int VEL_ATAQUE_MAX = 100;

    void _busca(Drive &motores, bool viuEsq, bool viuDir);
    void _ataque(Drive &motores);
};
