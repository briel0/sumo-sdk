#pragma once

#include "Mechanism.hpp"
#include <Arduino.h>
#include <ESP32Servo.h>

/**
    @class ContinuousServoMechanism
    @brief Servo de rotação contínua (360, ex.: MG90 360). Aqui o valor escrito no servo não é posição, é
   velocidade e sentido: perto de 90 fica parado, quanto mais longe de 90 mais rápido gira, pra um lado ou pro
   outro. Armar/recolher vira "gira nesse comando por X ms e corta o sinal".

    Não tem realimentação de posição: o quanto a peça anda depende do tempo e da tensão da bateria, e o erro
   acumula a cada ciclo de armar/recolher se a peça não tiver batente.
*/
class ContinuousServoMechanism : public Mechanism {
  public:
    /**
    @param pin        GPIO do sinal do servo.
    @param retractCmd Comando (0 a 180) usado pra recolher.
    @param deployCmd  Comando (0 a 180) usado pra armar.
    @param retractMs  Por quanto tempo girar pra recolher.
    @param deployMs   Por quanto tempo girar pra armar.
    */
    ContinuousServoMechanism(int pin, int retractCmd, int deployCmd, unsigned long retractMs, unsigned long deployMs);

    /**
    @brief Reserva os timers, recolhe (gira no comando de recolher por retractMs, bloqueando) e deixa a linha de
   sinal em LOW. Como não há realimentação de posição, o giro acontece mesmo se a peça já estiver recolhida —
   sem batente ela passa do ponto.
    */
    void init() override;

    void deploy() override;
    void retract() override;

    /**
    @brief Corta o sinal quando o tempo do giro atual acaba.
    */
    void update() override;

  private:
    void spin(int cmd, unsigned long durationMs);
    void stop();

    Servo _servo;
    int _pin;
    int _retractCmd;
    int _deployCmd;
    unsigned long _retractMs;
    unsigned long _deployMs;

    bool _spinning = false;
    unsigned long _spinStart = 0;
    unsigned long _spinDurationMs = 0;
};
