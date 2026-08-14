#pragma once

class Drive;
class WeaponSystem;
class HardwareCore;

/**
    @class CombatStrategy
    @brief Interface que define o contrato de combate autônomo.

    O AutoMode não sabe qual robô está lutando — só sabe chamar
    init() uma vez e autoEngage() a cada frame.

    Cada robô com modo autônomo implementa essa interface
    com sua própria lógica de busca e ataque. Os sensores e periféricos de
    estado vivem no HardwareCore (Core 0): a estratégia recebe estados lógicos
    já limpos e só decide o comportamento, comandando os motores no Core 1.
*/
class CombatStrategy {
  public:
    virtual ~CombatStrategy() = default;

    /**
    @brief Inicializa o estado interno da estratégia.
    Chamado uma única vez pelo AutoMode ao entrar em combate.
    @param hardware Núcleo de hardware (Core 0) para consultar sensores e
                    definir intenções de periféricos.
    */
    virtual void init(HardwareCore &hardware) = 0;

    /**
    @brief Executa um frame da lógica de combate.
    Chamado repetidamente pelo AutoMode enquanto o robô está lutando.
    @param motores  Referência aos motores (comandados diretamente no Core 1).
    @param armas    Referência ao sistema de armas.
    @param hardware Estados limpos dos sensores e intenções dos periféricos.
    */
    virtual void autoEngage(Drive &motores, WeaponSystem &armas, HardwareCore &hardware) = 0;
};
