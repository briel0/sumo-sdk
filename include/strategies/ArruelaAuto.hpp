#pragma once
#include "CombatStrategy.hpp"
#include "JS40F.hpp"
#include "MotionPlayer.hpp"
#include "RobotTypes.hpp"
#include "ToFSensor.hpp"

class ArruelaAuto : public CombatStrategy {
  public:
    ArruelaAuto();
    void init() override;
    void configure(const AutoStrategy &cfg) override;
    void autoEngage(Drive &motores, WeaponSystem &armas) override;
    String getSensorStatusJSON() override;

  private:
    // Apenas declarando a existência deles.
    JS40F _sensorEsq;
    JS40F _sensorDir;
    JS40F _sensorFrontal;
    ToFSensor _sensorDistancia;

    MotionPlayer _player;

    Direction _ultimoLado = Direction::left;

    // Ids de busca que chegam do site (AutoStrategy::search).
    static constexpr int BUSCA_PADRAO = 1;
    static constexpr int BUSCA_LENTA = 2;
    static constexpr int BUSCA_TOF = 3;

    static constexpr int VEL_BUSCA_GIRO = 90;  // BUSCA PADRAO: varre rapido
    static constexpr int VEL_BUSCA_LENTA = 35; // BUSCA LENTA: demora em cima do alvo
    static constexpr int VEL_ATAQUE_MAX = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 60;

    // BUSCA_TOF: so ataca com o VL53L0X enxergando alvo abaixo deste limiar.
    static constexpr uint16_t LIMIAR_TOF_MM = 100;
    // Espacamento entre leituras do ToF. Precisa ser maior que o orcamento de
    // 20ms configurado no ToFSensor, senao a leitura gira no I2C esperando amostra.
    static constexpr unsigned long INTERVALO_TOF_MS = 25;

    unsigned long _ultimaLeituraToF = 0;
    bool _toFViuAlvo = false;
    // Resultado do init() do VL53L0X. Se o sensor nao subiu, a BUSCA_TOF nunca
    // atacaria (todo temOponente() daria timeout), entao o configure() a recusa.
    bool _toFOk = false;

    // Cada busca do site e uma funcao propria com esta assinatura, e ela recebe o
    // frame inteiro — inclusive viuFrente. Isso e de proposito: quando atacar e
    // decisao do modo, nao regra global, e e o que deixa a BUSCA_TOF ignorar o
    // JS40F frontal. O configure() escolhe a funcao uma unica vez; o autoEngage()
    // so desreferencia o ponteiro, entao nao existe switch de id dentro do loop.
    using BuscaFn = void (ArruelaAuto::*)(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
    BuscaFn _buscaAtual = &ArruelaAuto::_buscaPadrao;

    void _buscaPadrao(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
    void _buscaLenta(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
    void _buscaToF(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);

    void _ataque(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
};