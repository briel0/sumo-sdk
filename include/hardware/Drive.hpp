#pragma once

// TO DO: Migrar para MCPWM V2 no futuro

// sdkconfig.h define CONFIG_IDF_TARGET_ESP32C3. Incluído explicitamente aqui
// porque este header costuma ser incluído ANTES de <Arduino.h> (que normalmente
// puxaria o sdkconfig) — sem isto, o #if abaixo não veria o alvo e os membros do
// Drive-LEDC não seriam declarados, mesmo o .cpp compilando o ramo C3.
#include "sdkconfig.h"

// O ESP32-C3 (Fumacinha RC) não tem o periférico MCPWM — só LEDC. Nesse alvo o
// Drive usa 4 canais LEDC (via ESP32PWM, pra coordenar a alocação com os servos,
// que usam os outros 2 dos 6 canais do C3). Nos demais alvos (ESP32 clássico)
// segue usando MCPWM direto. A interface pública é idêntica nos dois casos.
#if defined(CONFIG_IDF_TARGET_ESP32C3)
#include <ESP32Servo.h> // ESP32PWM
#endif

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

#if defined(CONFIG_IDF_TARGET_ESP32C3)
    // Um canal LEDC por entrada da ponte H. Guardamos os pinos e prendemos os
    // canais de forma preguiçosa (no primeiro uso, já dentro do loop) pra não
    // mexer no LEDC durante a inicialização estática dos globais.
    int      _rightPosPin, _rightNegPin, _leftPosPin, _leftNegPin;
    bool     _attached = false;
    ESP32PWM _pwmRightPos, _pwmRightNeg, _pwmLeftPos, _pwmLeftNeg;

    void _ensureAttached();
    // Aciona um par (IN1/IN2) de um motor: velocidade >0 avança (IN1=duty,IN2=0),
    // <0 ré (IN1=0,IN2=duty), ==0 nada (o brake/release cuidam dos dois pinos).
    void _drivePair(ESP32PWM &posCh, ESP32PWM &negCh, int speed);
#endif
};