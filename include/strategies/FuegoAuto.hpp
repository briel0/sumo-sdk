#pragma once
#include "CombatStrategy.hpp"
#include "RobotTypes.hpp"

class Drive;
class WeaponSystem;
class HardwareCore;

/**
    @class FuegoAuto
    @brief Combate autônomo do Fuego: frentão na largada + busca simples.

    Mesma forma do SmokerAuto/ArruelaAuto (busca girando, ataque fechando o
    arco), adaptada ao que o Fuego REALMENTE tem em hardware:

      - dois JSumo laterais (esquerda/direita). NÃO existe sensor frontal:
        PIN_JSUMO_ASA divide o pino 32 com o servo da asa, então
        HardwareCore::frontDetected() é lixo neste robô e esta estratégia
        nunca o consulta. O papel de "alvo centrado à frente" é feito pelos
        dois laterais acusando ao mesmo tempo.
      - LDR de rampa, com prioridade absoluta: oponente embarcado -> empurra.
      - servo da asa, aberto à ESQUERDA no primeiro frame de combate e
        mantido lá o resto da luta.

    Os sensores de linha do perfil (PIN_LINHA_ESQ/DIR) existem no hardware e o
    HardwareCore continua lendo, mas o Fuego NÃO os usa em combate por decisão
    de projeto: nenhum recuo de borda aqui. Quem tira o robô da borda é o
    piloto/o próprio ataque — a estratégia não olha lineLeft()/lineRight().

    O "frentão" não mora aqui: quem toca a MACRO_FRENTAO do perfil na largada
    é o AutoMode (MotionPlayer). Esta estratégia só recebe o frame depois que
    o saque cego termina — daí em diante é busca/ataque a cada frame.
*/
class FuegoAuto : public CombatStrategy {
  public:
    FuegoAuto() = default;

    void init(HardwareCore &hardware) override;
    void autoEngage(Drive &motores, WeaponSystem &armas, HardwareCore &hardware) override;

  private:
    // Último lado em que o oponente foi visto. É o palpite da busca quando os
    // dois laterais ficam cegos: gira pra onde ele estava, em vez de sortear.
    Direction _ultimoLado = Direction::left;

    // A asa é aberta no PRIMEIRO frame de combate, não no init(): o init() roda
    // ao entrar no modo AUTO (ainda na seleção de tática pelo celular), e o
    // pedido é que ela abra no início da LUTA. Resetado no init() para que uma
    // segunda luta sem reboot volte a abrir.
    bool _asaAberta = false;

    // Latch só de log do ataque por LDR — evita afogar o serial a cada frame.
    // Não participa da decisão de atacar, que é tomada da leitura, todo frame.
    bool _rampaAtacando = false;

    static constexpr int VEL_BUSCA_GIRO      = 80;
    static constexpr int VEL_ATAQUE_MAX      = 100;
    static constexpr int VEL_ATAQUE_REDUZIDA = 50;
    static constexpr int VEL_RAMPA           = 100; // empurrão com o oponente na rampa

    /**
    @brief Cegueira total: gira no próprio eixo pro último lado visto.
    */
    void _busca(Drive &motores);

    /**
    @brief Alvo à vista em pelo menos um lateral. Dois laterais = alvo centrado,
           avanço reto em força máxima; um só = arco fechando pra aquele lado
           (avança E gira, para não depender dos dois feixes se cruzarem).
    */
    void _ataque(Drive &motores, bool viuEsq, bool viuDir);
};
