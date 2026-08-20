#pragma once
#include <Arduino.h>

class Drive;
class WeaponSystem;

/**
    @class CombatStrategy
    @brief Interface que define o contrato de combate autônomo.

    O AutoMode não sabe qual robô está lutando — só sabe chamar
    init() uma vez e executarCombate() a cada frame.

    Cada robô com modo autônomo implementa essa interface
    com seus próprios sensores, lógica de busca e ataque.
*/
class CombatStrategy {
  public:
    virtual ~CombatStrategy() = default;

    /**
    @brief Inicializa os sensores e estado interno da estratégia.
    Chamado uma única vez pelo AutoMode ao entrar em combate.
    */
    virtual void init() = 0;

    /**
    @brief Executa um frame da lógica de combate.
    Chamado repetidamente pelo AutoMode enquanto o robô está lutando.
    @param motores  Referência aos motores.
    @param armas    Referência ao sistema de armas.
    */
    virtual void autoEngage(Drive &motores, WeaponSystem &armas) = 0;

    /**
    @brief Retorna o estado atual dos sensores do robô no formato JSON.
    Pode ser sobrescrito por cada robô para expor seus sensores na interface web de debug.
    */
    virtual String getSensorStatusJSON() {
        return "{}";
    }
};