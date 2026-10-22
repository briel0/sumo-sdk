#pragma once
#include "CombatStrategy.hpp"
#include "LDR.hpp"
#include "MotionPlayer.hpp"
#include "RobotTypes.hpp"
#include "ToFSensor.hpp"

class CaiporaAuto : public CombatStrategy {
  public:
    static constexpr int PIN_LDR = 39;
    static constexpr int PIN_XSHUT_FRENTE_ESQ = 15;
    static constexpr int PIN_XSHUT_LATERAL_ESQ = 4;
    static constexpr int PIN_XSHUT_FRENTE_DIR = 27;
    static constexpr int PIN_XSHUT_LATERAL_DIR = 14;

    CaiporaAuto();
    void init() override;
    void configure(const AutoStrategy &cfg) override;
    void autoEngage(Drive &motores, WeaponSystem &armas) override;
    String getSensorStatusJSON() override;

  private:
    LDR _ldr;
    ToFSensor _vlFrenteEsq;
    ToFSensor _vlLateralEsq;
    ToFSensor _vlFrenteDir;
    ToFSensor _vlLateralDir;
    MotionPlayer _player;

    Direction _ultimoLado = Direction::left;

    // Resultado do init() de cada VL53L0X (igual _toFOk do ArruelaAuto). Se um
    // sensor não subiu (ou nunca vai subir por crosstalk entre os 4 no mesmo
    // barramento), a busca/telemetria ignora ele em vez de chamar
    // temOponente()/leituraRaw() num sensor morto e arriscar o timeout de
    // 500ms (ToFSensor::init()) travando o autoEngage() inteiro.
    bool _frenteEsqOk = false;
    bool _lateralEsqOk = false;
    bool _frenteDirOk = false;
    bool _lateralDirOk = false;

    // Busca padrão igual à do Arruela: os VL laterais fazem o papel dos JS40F
    // esq/dir (giram o robô na direção de quem viu algo), e o LDR faz o papel do
    // JS40F frontal — abaixo do limiar (algo bloqueando a luz de cima) é alvo na
    // cara, ataca com tudo.
    static constexpr uint16_t LIMIAR_BUSCA_MM = 250;
    static constexpr uint16_t LIMIAR_LDR = 300;
    static constexpr int VEL_BUSCA_CUIDADOSA = 40;
    static constexpr int VEL_ATAQUE = 100;

    // Espaçamento entre leituras dos VL. readRangeContinuousMillimeters() gira no
    // I2C até sair amostra nova — ler os quatro todo frame prenderia o loop.
    static constexpr unsigned long INTERVALO_TOF_MS = 25;
    unsigned long _ultimaLeituraToF = 0;

    // Última distância raw (mm) de cada VL. SEM_LEITURA_MM enquanto não leu ou
    // se o sensor falhou no init() — fica acima de qualquer limiar de detecção.
    static constexpr uint16_t SEM_LEITURA_MM = UINT16_MAX;
    uint16_t _rawFrenteEsq = SEM_LEITURA_MM;
    uint16_t _rawLateralEsq = SEM_LEITURA_MM;
    uint16_t _rawFrenteDir = SEM_LEITURA_MM;
    uint16_t _rawLateralDir = SEM_LEITURA_MM;

    // Ids de busca que chegam do site (AutoStrategy::search).
    // As OFENSIVAS sao as mesmas buscas; so muda o que o robo faz sem alvo
    // nenhum a vista (ver _velSemAlvo).
    static constexpr int BUSCA_RAPIDA = 2; // RAPIDA DEFENSIVA
    static constexpr int BUSCA_LENTA = 3;  // LENTA DEFENSIVA
    static constexpr int BUSCA_RAPIDA_OFENSIVA = 4;
    static constexpr int BUSCA_LENTA_OFENSIVA = 5;

    // Sem alvo a vista: a DEFENSIVA fica parada esperando (0), a OFENSIVA
    // avanca reto nessa velocidade. Escolhido no configure().
    static constexpr int VEL_OFENSIVA = 50;
    int _velSemAlvo = 0;

    // Mesmo esquema do ArruelaAuto: o configure() escolhe a função uma única vez
    // e o autoEngage() só desreferencia o ponteiro.
    using BuscaFn = void (CaiporaAuto::*)(Drive &motores, uint16_t rawFrenteEsq, uint16_t rawLateralEsq,
                                          uint16_t rawFrenteDir, uint16_t rawLateralDir, int rawLdr);
    BuscaFn _buscaAtual = &CaiporaAuto::_buscaRapida;

    void _buscaRapida(Drive &motores, uint16_t rawFrenteEsq, uint16_t rawLateralEsq, uint16_t rawFrenteDir,
                      uint16_t rawLateralDir, int rawLdr);
    void _buscaLenta(Drive &motores, uint16_t rawFrenteEsq, uint16_t rawLateralEsq, uint16_t rawFrenteDir,
                     uint16_t rawLateralDir, int rawLdr);
};
