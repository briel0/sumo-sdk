#pragma once
#include <Arduino.h>

class LDR {
  private:
    uint8_t _pin;

  public:
    /**
     * @brief Construtor do sensor LDR.
     * @param pin Pino analógico (ADC) onde o divisor de tensão do LDR está conectado.
     */
    explicit LDR(uint8_t pin);

    /**
     * @brief Configura o pino como entrada. Deve ser chamado no init() do robô.
     */
    void init();

    /**
     * @brief Retorna o valor cru do ADC (0 a 4095 no ESP32).
     */
    int readRaw() const;

    /**
     * @brief Retorna a luminosidade mapeada em percentagem (0 a 100%).
     * @note Assume um circuito divisor de tensão onde mais luz = maior voltagem.
     */
    int getPercentage() const;

    /**
     * @brief Compara a leitura com um limite (threshold) para atuar como sensor digital.
     * @param threshold Limite de corte (0-4095).
     * @return true se a leitura for menor que o limite (ambiente escuro).
     */
    bool isDark(int threshold = 1500) const;
};