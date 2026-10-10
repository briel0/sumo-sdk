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

    // Toca o giro inicial e as fugas de borda. E o unico dono dos motores
    // enquanto estiver ativo.
    MotionPlayer _player;

    // Ids de busca que chegam do site — aqui reaproveitados como escolha do
    // delay pra abrir a asa (unica config do Marola).
    static constexpr int ASA_DELAY_100 = 1;
    static constexpr int ASA_DELAY_500 = 2;
    static constexpr int ASA_DELAY_100_GIRO = 3;
    static constexpr int BUSCA_LINHA = 4;

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
    // main.cpp) — sem lógica de busca, fora a espera pra abrir a asa, a fuga
    // de borda pelos QRE1113 (MACRO_FUGA_LINHA_*, via _player) e o ataque
    // pelos LDRs (LIMIAR_LDR).
    static constexpr int VEL_ATAQUE_MAX = 100;

    // Qualquer LDR abaixo disso (algo bloqueando a luz) dispara o avanco a
    // VEL_ATAQUE_MAX.
    static constexpr int LIMIAR_LDR = 100;

    // BUSCA_LINHA: avanca devagar o tempo todo, deixando a fuga de borda
    // decidir pra onde ir. A asa abre igual ao ASA_DELAY_100.
    static constexpr int VEL_BUSCA_LINHA = 55;

    // Velocidade do avanco reto, escolhida no configure() conforme a busca.
    int _velAvanco = VEL_ATAQUE_MAX;
};
