#pragma once
#include <Arduino.h>

/**
    @class TSSP4038
    @brief Driver digital para o receptor infravermelho Vishay TSSP4038.

    Uso PASSIVO: o robô não emite nada por este sensor. O TSSP4038 demodula a
    portadora de 38kHz, então o que ele enxerga é o IR modulado que o sensor do
    ADVERSÁRIO (ex.: JSUMO) está emitindo pra nos procurar.

    A saída opera com lógica invertida (Active LOW): com portadora de 38kHz
    chegando, vai para GND (0V); sem sinal, o pull-up interno mantém em VCC.

    A leitura é instantânea, sem filtro nem retenção: se o emissor do adversário
    trabalhar em rajadas, temSinal() pisca junto.
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
    @brief Retorna true se há IR modulado a 38kHz chegando neste instante.
           Resolve automaticamente a inversão de lógica (Active LOW).
    */
    bool temSinal() const;

    /**
    @brief Retorna o estado lógico cru (0 ou 1) da porta digital.
           Útil para telemetria de bancada ou debug no Serial Monitor.
    */
    bool leituraRaw() const;

  private:
    uint8_t _pin;
};
