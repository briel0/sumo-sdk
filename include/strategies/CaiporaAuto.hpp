#pragma once
#include "CombatStrategy.hpp"
#include "LDR.hpp"
#include "MotionPlayer.hpp"
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

    // Auto simplificado: anda pra frente até o STOP do IR (botão 3), que já é
    // tratado globalmente em main.cpp (reinicia o ESP32) — nenhuma lógica de
    // busca/ataque aqui.
    static constexpr int VEL_ATAQUE = 100;
};