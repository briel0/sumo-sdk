#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

/**
    @class ServoMechanism
    @brief Encapsulates the ESP32Servo library to prevent hardware timer conflicts and provides clean methods for
   deploying mechanisms (like ramps or flags).
*/
class ServoMechanism {
  public:
    /**
    @brief Constructs the mechanism and stores the hardware pin.
    @param pin The GPIO pin connected to the servo's signal wire.
    */
    ServoMechanism() = default;
    ServoMechanism(int pin, int retractAngle = 0, int deployAngle = 90);

    /**
    @brief Allocates ESP32 hardware timers and attaches the servo.
    */
    void init();

    /**
    @brief Moves the servo to a specific angle safely.
    @param angle Target angle (0 to 180 degrees).
    */
    void setAngle(int angle);

    void deploy();
    void retract();
    void relax();

    /**
    @brief Anda delta graus a partir do ângulo atual, preso entre
           retractAngle e deployAngle. delta > 0 anda rumo ao deployAngle,
           delta < 0 rumo ao retractAngle — vale mesmo com deploy < retract.
    */
    void nudge(int delta);

  private:
    /**
    @brief The underlying ESP32Servo instance used to control hardware PWM.
    */
    Servo _servo;

    /**
    @brief The GPIO pin connected to the servo's signal wire.
    */
    int _pin;

    /**
    @brief The angle (in degrees) when the mechanism is fully retracted/disarmed.
    */
    int _retractAngle;

    /**
    @brief The angle (in degrees) when the mechanism is fully deployed/armed.
    */
    int _deployAngle;

    /**
    @brief Último ângulo escrito no servo — base do nudge().
    */
    int _angle = 0;
};