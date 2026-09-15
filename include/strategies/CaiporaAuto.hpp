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
    static constexpr uint16_t LIMIAR_LDR = 4000;
    static constexpr int VEL_BUSCA_CUIDADOSA = 40;
    static constexpr int VEL_ATAQUE = 100;

    // Espaçamento entre leituras dos VL laterais. readRangeContinuousMillimeters()
    // gira no I2C até sair amostra nova — ler os dois todo frame prenderia o loop.
    static constexpr unsigned long INTERVALO_TOF_MS = 25;
    unsigned long _ultimaLeituraToF = 0;
    bool _viuLateralEsq = false;
    bool _viuLateralDir = false;

    void _busca(Drive &motores, bool viuEsq, bool viuDir, bool viuFrente);
};
