#pragma once
#include <Arduino.h>

/**
    @class TSSP4038
    @brief Driver digital para o receptor infravermelho Vishay TSSP4038.

    Lê a saída do sensor de proximidade. O TSSP4038 opera com lógica invertida
    (Active LOW). Quando o reflexo do IR modulado a 38kHz é detectado, a saída
    vai para GND (0V). Quando não há obstáculo, o pull-up interno mantém em VCC (3.3V/5V).
*/
class TSSP4038 {
  public:
    /**
    @brief Constrói o sensor associado a um pino digital.
    @param pin Pino digital GPIO conectado à saída (OUT) do sensor.
    */
    explicit TSSP4038(uint8_t pin);

    /**
    @brief Configura o pino como entrada.
    */
    void init() const;

    /**
    @brief Retorna true se um obstáculo (oponente) foi detectado.
           Resolve automaticamente a inversão de lógica (Active LOW).
    */
    bool temOponente() const;

    /**
    @brief Retorna o estado lógico cru (0 ou 1) da porta digital.
           Útil para telemetria de bancada ou debug no Serial Monitor.
    */
    bool leituraRaw() const;

  private:
    uint8_t _pin;
};