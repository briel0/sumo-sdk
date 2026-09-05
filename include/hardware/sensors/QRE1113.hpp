
#pragma once
#include <Arduino.h>

/**
    @class QRE1113
    @brief Driver para o sensor de linha reflexivo QRE1113.

    Lê o valor analógico do fototransistor e compara com um limiar
    para detectar superfície branca (pista) vs escura (robô/sombra).

    A direção da comparação (branco = ADC maior ou menor) depende da
    montagem/fiação de cada robô — não assuma, confirme com leituraRaw()
    no painel /sensors antes de calibrar o threshold.
*/
class QRE1113 {
  public:
    /**
    @brief Constrói o sensor associado a um pino analógico.
    @param pin Pino analógico GPIO conectado à saída do QRE1113.
    @param threshold Valor ADC (0–4095) abaixo do qual a superfície é considerada branca
                     (ver leituraRaw() pra calibrar por robô).
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