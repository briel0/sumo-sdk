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
};