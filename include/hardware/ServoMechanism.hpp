#pragma once

#include "Mechanism.hpp"
#include <Arduino.h>
#include <ESP32Servo.h>

/**
    @class ServoMechanism
    @brief Encapsulates the ESP32Servo library to prevent hardware timer conflicts and provides clean methods for
   deploying mechanisms (like ramps or flags). Servo de POSIÇÃO — pra servo de rotação contínua (360) ver
   ContinuousServoMechanism.
*/
class ServoMechanism : public Mechanism {
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
    void init() override;

    /**
    @brief Moves the servo to a specific angle safely.
    @param angle Target angle (0 to 180 degrees).
    */
    void setAngle(int angle) override;

    void deploy() override;
    void retract() override;

    /**
    @brief Corta o sinal RELAX_TIMEOUT_MS depois de armar, pra o servo não ficar forçando contra a peça.
    */
    void update() override;

    void relax();

  private:
    static constexpr unsigned long RELAX_TIMEOUT_MS = 1000;

    void write(int angle);

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
    @brief True enquanto o relax está agendado (depois de um deploy/setAngle). Recolhido o servo fica segurando.
    */
    bool _relaxPending = false;
    unsigned long _relaxTimerStart = 0;
};
