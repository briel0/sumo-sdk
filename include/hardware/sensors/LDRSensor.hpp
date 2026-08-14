#pragma once
#include <Arduino.h>

/**
    @class LDRSensor
    @brief Leitura do LDR de confirmação de alavanca/rampa (divisor de tensão
           com resistor de 10k).

    Aplica um filtro de média móvel para atenuar o ruído elétrico dos motores
    e compara o valor suavizado com um limiar ajustável. Quando o adversário
    projeta sombra sobre a rampa, a luminosidade cai bruscamente e o valor do
    ADC despenca abaixo do limiar — belowThreshold() sinaliza a detecção.
*/
class LDRSensor {
  public:
    // Teto do buffer estático da média móvel (janela configurável até aqui).
    static constexpr uint8_t MAX_WINDOW = 32;

    /**
    @brief Constrói o sensor associado a um pino analógico.
    @param pin        GPIO analógico conectado ao divisor LDR/10k.
    @param threshold  Valor ADC (0–4095) abaixo do qual considera-se rampa ocupada.
    @param windowSize Número de amostras da média móvel (1–MAX_WINDOW).
    */
    explicit LDRSensor(uint8_t pin, uint16_t threshold, uint8_t windowSize = 8);

    /**
    @brief Configura o pino como entrada e prime o filtro com a leitura atual.
    */
    void init();

    /**
    @brief Coleta uma amostra do ADC e alimenta a média móvel.
           Deve ser chamada pela task de alta frequência do Core 0.
    */
    void update();

    /**
    @brief Último valor filtrado (suavizado) do ADC, 0–4095. Útil para calibrar.
    */
    uint16_t filtered() const;

    /**
    @brief true quando o valor filtrado caiu abaixo do limiar.
    */
    bool belowThreshold() const;

  private:
    uint8_t  _pin;
    uint16_t _threshold;
    uint8_t  _window;

    uint16_t _samples[MAX_WINDOW] = {0};
    uint8_t  _index    = 0;
    uint32_t _sum      = 0;
    uint16_t _filtered = 0;
};
