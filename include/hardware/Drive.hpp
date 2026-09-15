#pragma once
#include "MotorPolarity.hpp"

// TO DO: Migrar para MCPWM V2 no futuro

/**
    @class Drive
    @brief Core logic to control the robot's movement and A4950 motor drivers.
*/

class Drive {
  public:
    /**
    @brief Constructs the Drive object and initializes the MCPWM hardware timers.
    @param rightPosPin GPIO pin connected to IN1 of the right motor.
    @param rightNegPin GPIO pin connected to IN2 of the right motor.
    @param leftPosPin GPIO pin connected to IN1 of the left motor.
    @param leftNegPin GPIO pin connected to IN2 of the left motor.
    */
    Drive(int rightPosPin, int rightNegPin, int leftPosPin, int leftNegPin);

    /**
    @brief Carrega da NVS o transform de polaridade salvo (inverter/trocar
           lado). Chamar uma vez no setup(), nunca no construtor: Drive é
           instanciado como objeto global, antes do Arduino garantir a NVS
           pronta — só o construtor bind os pinos GPIO, que não depende disso.
    */
    void initPolarity() {
        _polarity.init();
    }

    /**
    @brief Avança pro próximo transform de polaridade (0..7) e persiste.
           Pensado pra ser chamado por um botão de bancada (site no AUTO,
           combo no RC) — o operador vê o efeito no próximo setSpeed().
    */
    uint8_t cyclePolarity() {
        return _polarity.cycle();
    }

    uint8_t currentPolarity() const {
        return _polarity.current();
    }

    /**
    @brief Sets the speed and direction for both motors.
    @param leftSpeed Percentage of power for the left motor (-100 to +100).
    @param rightSpeed Percentage of power for the right motor (-100 to +100).
    */
    void setSpeed(int leftSpeed, int rightSpeed);

    /**
    @brief Applies active braking to both motors.
    */
    void brake();

    /**
    @brief Cuts power to the motors, allowing them to spin freely.
    */
    void release();

  private:
    /**
    @brief Stores the last applied speed for the left motor. Used to optimize PWM updates and detect direction changes.
    */
    int _lastLeftSpeed = 0;

    /**
    @brief Stores the last applied speed for the right motor. Used to optimize PWM updates and detect direction changes.
    */
    int _lastRightSpeed = 0;

    /**
    @brief Internal helper to send the calculated PWM duty cycle to the hardware timers.
    @param leftPWM Duty cycle percentage for the left motor (-100 to 100).
    @param rightPWM Duty cycle percentage for the right motor (-100 to 100).
    */
    void _applyPWM(int leftPWM, int rightPWM);

    MotorPolarity _polarity;
};