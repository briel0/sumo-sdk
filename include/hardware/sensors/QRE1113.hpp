
#pragma once
#include <Arduino.h>

/**
    @class QRE1113
    @brief Driver para o sensor de linha reflexivo QRE1113.

    Lê o valor analógico do fototransistor e compara com um limiar
    para detectar superfície branca (pista) vs escura (robô/sombra).

    Superfície branca reflete mais luz → menor resistência → tensão mais alta → valor ADC maior.
    O limiar padrão é calibrado para o dojo de competição (superfície branca sobre fundo preto).
*/
class QRE1113 {
  public:
    /**
    @brief Constrói o sensor associado a um pino analógico.
    @param pin Pino analógico GPIO conectado à saída do QRE1113.
    @param threshold Valor ADC (0–4095) acima do qual a superfície é considerada branca.
                     Padrão de 2800 calibrado para o dojo padrão de competição.
    */
    explicit QRE1113(uint8_t pin, uint16_t threshold = 2800);

    /**
    @brief Configura o pino como entrada analógica.
    */
    void init() const;

    /**
    @brief Retorna true se o sensor está sobre a linha branca.
    */
    bool temLinhaBranca() const;

    /**
    @brief Retorna o valor ADC bruto da última leitura (0–4095).
           Útil para calibração do threshold em campo.
    */
    uint16_t leituraRaw() const;

  private:
    uint8_t  _pin;
    uint16_t _threshold;
};