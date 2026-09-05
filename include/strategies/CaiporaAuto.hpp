#pragma once
#include "CombatStrategy.hpp"
#include "LDR.hpp"
#include "MotionPlayer.hpp"
#include "QRE1113.hpp"
#include "RobotTypes.hpp"
#include "ToFSensor.hpp"

class CaiporaAuto : public CombatStrategy {
  public:
    static constexpr int PIN_LDR = 39;
    static constexpr int PIN_XSHUT_FRENTE_ESQ = 15;
    static constexpr int PIN_XSHUT_LATERAL_ESQ = 4;
    static constexpr int PIN_XSHUT_FRENTE_DIR = 27;
    static constexpr int PIN_XSHUT_LATERAL_DIR = 14;
    static constexpr int PIN_LINHA_ESQ = 15;
    static constexpr int PIN_LINHA_DIR = 36;

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

    // Busca por distância: só os dois VL frontais decidem, cuidadosa de
    // propósito (velocidades baixas) — reage cedo (25cm) e gira pouco em vez
    // de atacar forte feito o Arruela.
    static constexpr uint16_t LIMIAR_BUSCA_MM = 250;
    static constexpr int VEL_BUSCA_CUIDADOSA = 40;
    static constexpr int VEL_BUSCA_FRENTE = 50;

    // Espaçamento entre leituras dos VL frontais. readRangeContinuousMillimeters()
    // gira no I2C até sair amostra nova — ler os dois todo frame prenderia o
    // loop (e atrasaria a checagem da linha branca, que vem antes).
    static constexpr unsigned long INTERVALO_TOF_MS = 25;
    unsigned long _ultimaLeituraToF = 0;
    bool _viuFrenteEsq = false;
    bool _viuFrenteDir = false;

    void _busca(Drive &motores, bool viuFrenteEsq, bool viuFrenteDir);
};