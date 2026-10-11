#pragma once
#include "CombatStrategy.hpp"
#include "JS40F.hpp"
#include "MotionPlayer.hpp"
#include "QRE1113.hpp"
#include "RobotTypes.hpp"
#include "TSSP4038.hpp"

class SmokerAuto : public CombatStrategy {
  public:
    SmokerAuto();
    void init() override;
    void configure(const AutoStrategy &cfg) override;
    void autoEngage(Drive &motores, WeaponSystem &armas) override;
    bool alvoNaEspera() override;
    String getSensorStatusJSON() override;

  private:
    // Arco frontal: tres JS40F digitais.
    JS40F _sensorEsq;
    JS40F _sensorDir;
    JS40F _sensorFrontal;

    // Receptores passivos: enxergam o IR que o sensor do adversario emite.
    // So entram na decisao na BUSCA IR; na PADRAO ficam so pro
    // getSensorStatusJSON().
    TSSP4038 _irEsq;
    TSSP4038 _irDir;
    TSSP4038 _irFrontal;

    // Borda do dojo: dois QRE1113 analogicos, um por lado da frente.
    QRE1113 _linhaEsq;
    QRE1113 _linhaDir;

    // Toca as fugas de borda. E o unico dono dos motores enquanto estiver
    // ativo, inclusive por cima da busca escolhida no site.
    MotionPlayer _player;

    Direction _ultimoLado = Direction::left;

    // Ids de busca que chegam do site (AutoStrategy::search).
    static constexpr int BUSCA_PADRAO = 1;
    static constexpr int BUSCA_IR = 2;
    static constexpr int BUSCA_PINBALL = 3;

    // BUSCA IR: mesma logica da PADRAO, mas quem decide sao os tres TSSP4038
    // no lugar dos JS40F, e o transistor do PIN_JS_POWER fica desligado. O
    // configure() resolve isso uma unica vez, antes da largada.
    bool _buscaIr = false;

    // BUSCA PINBALL: usa os JS40F como a PADRAO; so troca a rotina de busca.
    bool _pinball = false;

    // Nao arma nada aqui: quem abre a arma e o AutoMode na largada, conforme
    // a config do site. Decide busca/ataque frame a frame, igual a
    // BUSCA PADRAO do ArruelaAuto (mesma logica, so troca ToF/JS40F esq-dir
    // pelos 3 JS40F). ATAQUE: frontal viu alguem, avanca no talo sem girar.
    // BUSCA: lateral viu alguem, gira pra la; cego nos tres, gira pro ultimo
    // lado que viu alguem.
    static constexpr int VEL_BUSCA_GIRO = 80;
    static constexpr int VEL_ATAQUE_MAX = 100;

    void _busca(Drive &motores, bool viuEsq, bool viuDir);
    void _buscaPinball(Drive &motores, bool viuEsq, bool viuDir);
    void _ataque(Drive &motores);
};
