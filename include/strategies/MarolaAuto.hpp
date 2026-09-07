#pragma once
#include "CombatStrategy.hpp"
#include "MotionPlayer.hpp"
#include "RobotTypes.hpp"
#include "hardware/sensors/LDR.hpp"
#include "hardware/sensors/QRE1113.hpp"

class MarolaAuto : public CombatStrategy {
  public:
    MarolaAuto();
    void init() override;
    void configure(const AutoStrategy &cfg) override;
    void autoEngage(Drive &motores, WeaponSystem &armas) override;
    String getSensorStatusJSON() override;

  private:
    LDR _ldrEsq;
    LDR _ldrDir;
    LDR _ldrFront;
    QRE1113 _linhaEsq;
    QRE1113 _linhaDir;
    MotionPlayer _player;

    // Ids de busca que chegam do site — aqui reaproveitados como escolha do
    // delay pra abrir a asa (unica config do Marola).
    static constexpr int ASA_DELAY_100 = 1;
    static constexpr int ASA_DELAY_500 = 2;
    static constexpr int ASA_DELAY_100_GIRO = 3;

    unsigned long _atrasoAbrirAsaMs = 100;
    unsigned long _tempoEntradaCombate = 0;

    // 100ms: anda durante a espera. 500ms: bloqueante — fica parado até a
    // asa abrir e só então sai andando (ver autoEngage()).
    bool _bloqueiaAteAbrir = false;

    // ASA_DELAY_100_GIRO: aponta pra MACRO_GIRO_INICIAL_ESQUERDA/DIREITA (Config.hpp)
    // conforme cfg.direction — nullptr fora desse modo. Tocada uma vez ao
    // entrar em combate, antes do avanço reto — ver autoEngage().
    const MotionSequence *_giroInicialMacro = nullptr;

    // Auto simplificado: anda pra frente até o STOP do IR (botão 3, ver
    // main.cpp) — sem lógica de busca/ataque, fora a espera pra abrir a asa.
    // LDRs e QRE1113 continuam lidos só pro getSensorStatusJSON().
    static constexpr int VEL_ATAQUE_MAX = 100;
};
