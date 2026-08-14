#pragma once

#include "RobotTypes.hpp"
#include <Arduino.h>
#include <ESP32Servo.h>

/**
    @class WingServo
    @brief Servo da asa lateral com três posições fundamentais (recolhida, aberta
           à esquerda e aberta à direita). Encapsula um servo de hobby acionado
           pelo periférico LEDC do ESP32.

    Como usa a mesma infraestrutura do ServoMechanism/WeaponSystem, o attach e as
    escritas competem pelos mesmos timers/canais LEDC do sistema de armas — ambos
    hoje rodam síncronos no Core 1 (ver HardwareCore.hpp), então não há disputa
    entre núcleos, só entre chamadas no mesmo frame.
*/
class WingServo {
  public:
    WingServo() = default;
    WingServo(int pin, int retractAngle, int leftAngle, int rightAngle);

    /**
    @brief Aloca o timer LEDC, prende o servo e o move para a posição recolhida.
    */
    void init();

    /**
    @brief Move a asa para uma das três posições fundamentais.
    */
    void moveTo(WingPosition position);

    WingPosition position() const {
        return _position;
    }

  private:
    Servo        _servo;
    int          _pin          = -1;
    int          _retractAngle = 90;
    int          _leftAngle    = 0;
    int          _rightAngle   = 180;
    WingPosition _position     = WingPosition::RETRACTED;

    void _writeAngle(int angle);
};
