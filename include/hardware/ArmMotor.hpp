#pragma once

/**
    @class ArmMotor
    @brief N20 do braço do trator. Mesmo esquema de ponte H do Drive (IN1/IN2
           em PWM), só que no MCPWM_TIMER_2 da unidade 0 — os timers 0 e 1
           já são do Drive.
*/
class ArmMotor {
  public:
    /**
    @param posPin GPIO ligado no IN1 do driver do braço.
    @param negPin GPIO ligado no IN2 do driver do braço.
    */
    ArmMotor(int posPin, int negPin);

    /**
    @brief Potência do braço (-100 a +100). 0 freia — N20 com redução segura
           melhor o braço freado do que solto.
    */
    void setSpeed(int speed);

    void brake();
    void release();

  private:
    int _lastSpeed = 0;
};
