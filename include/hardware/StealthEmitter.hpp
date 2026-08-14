#pragma once

#include <Arduino.h>

/**
    @class StealthEmitter
    @brief Controla o transistor low-side que chaveia o GND dos emissores IR
           laterais ("modo furtivo").

    Desligar a emissão evita que o robô acenda os receptores IR passivos do
    adversário, negando informação de posição a ele. A FSM habilita ou desabilita
    a emissão à vontade através de enable()/disable(), sem saber da eletrônica.

    O pino aciona a base de um NPN em low-side: nível ALTO conduz o transistor,
    fecha o GND dos emissores e liga a emissão.
*/
class StealthEmitter {
  public:
    /**
    @brief Constrói o controle associado ao pino do transistor.
    @param pin GPIO conectado à base do transistor de chaveamento.
    */
    explicit StealthEmitter(int pin);

    /**
    @brief Configura o pino como saída. A emissão começa desligada (furtivo).
    */
    void init();

    /**
    @brief Liga os emissores IR laterais (transistor conduzindo, GND fechado).
    */
    void enable();

    /**
    @brief Desliga os emissores IR laterais (modo furtivo).
    */
    void disable();

    /**
    @brief Define o estado da emissão diretamente.
    @param on true liga a emissão, false ativa o modo furtivo.
    */
    void set(bool on);

    bool isEnabled() const {
        return _enabled;
    }

  private:
    int  _pin;
    bool _enabled = false;
};
