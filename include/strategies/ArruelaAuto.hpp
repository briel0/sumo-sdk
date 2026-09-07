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
    static constexpr int BUSCA_GATO_PRETO = 4;
    static constexpr int BUSCA_BALA_TENSA = 5;

    static constexpr int VEL_BUSCA_GIRO = 65;  // BUSCA PADRAO: varre rapido
    static constexpr int VEL_BUSCA_LENTA = 55; // BUSCA LENTA: demora em cima do alvo
    static constexpr int VEL_ATAQUE_MAX = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 60;

    // BUSCA_TOF: so ataca com o VL53L0X enxergando alvo abaixo deste limiar.
    static constexpr uint16_t LIMIAR_TOF_MM = 200;
    // Espacamento entre leituras do ToF. Precisa ser maior que o orcamento de
    // 20ms configurado no ToFSensor, senao a leitura gira no I2C esperando amostra.
    static constexpr unsigned long INTERVALO_TOF_MS = 25;

    unsigned long _ultimaLeituraToF = 0;
    bool _toFViuAlvo = false;
    // Resultado do init() do VL53L0X. Se o sensor nao subiu, a BUSCA_TOF nunca
    // atacaria (todo temOponente() daria timeout), entao o configure() a recusa.
    bool _toFOk = false;

    // BUSCA_GATO_PRETO: por esse tanto de tempo so alinha com os JS40F (sem
    // atacar pelo frontal) — a espera so quebra antes se o VL53L0X confirmar
    // alvo dentro do limiar, disparando o ataque na hora. Esgotada a janela
    // sem confirmacao, entrega a luta pra BUSCA_PADRAO.
    static constexpr unsigned long DURACAO_GATO_PRETO_MS = 2000;
    static constexpr uint16_t LIMIAR_GATO_PRETO_MM = 380;
    unsigned long _inicioGatoPreto = 0;

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
    void _buscaGatoPreto(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
    // BUSCA_BALA_TENSA: mesma deteccao da BUSCA_TOF (reaproveita LIMIAR_TOF_MM,
    // INTERVALO_TOF_MS e o cache _ultimaLeituraToF/_toFViuAlvo), mas em vez de
    // atacar reto na confirmacao ela dispara uma MACRO_CURVINHA (Config.hpp)
    // pro ultimo lado conhecido, via _player.
    void _buscaBalaTensa(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);

    void _ataque(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
};