#pragma once

/**
    @class Drive
    @brief Core logic to control the robot's movement and A4950 motor drivers.
 */

class Drive{
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
    int _lastLeftSpeed;
    int _lastRightSpeed;

    /**
    @brief Internal helper to send the calculated PWM duty cycle to the hardware timers.
    @param leftPWM Duty cycle percentage for the left motor (-100 to 100).
    @param rightPWM Duty cycle percentage for the right motor (-100 to 100).
    */
    void _applyPWM(int leftPWM, int rightPWM);

};