#include <Arduino.h>
#include <Drive.hpp>
#include <driver/mcpwm.h>

Drive::Drive(int rightPosPin, int rightNegPin, int leftPosPin, int leftNegPin) {

    _lastLeftSpeed = 0;
    _lastRightSpeed = 0;
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, leftPosPin);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, leftNegPin);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, rightPosPin);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, rightNegPin);

    mcpwm_config_t pwm_config;

    // Aparentemente é uma boa freq que mantém o silencio e o torque estável
    pwm_config.frequency = 20000;

    // Seta 0 de potencia
    pwm_config.cmpr_a = 0;
    pwm_config.cmpr_b = 0;

    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

    // Inicia os timers: Unidade 0, Timer 0 (Esq) e Timer 1 (Dir)
    // Basicamente, cada timer fica com um motor
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);

    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);

    release();
}

void Drive::setSpeed(int leftSpeed, int rightSpeed) {

    leftSpeed = constrain(leftSpeed, -100, 100);
    rightSpeed = constrain(rightSpeed, -100, 100);

    if(leftSpeed == _lastLeftSpeed && rightSpeed == _lastRightSpeed) {
        return;
    }

    if(leftSpeed == 0 && rightSpeed == 0) {
        brake();
        delayMicroseconds(25);
        _lastLeftSpeed = 0;
        _lastRightSpeed = 0;
        return;
    }

    bool leftInverted = (leftSpeed * _lastLeftSpeed) < 0;
    bool rightInverted = (rightSpeed * _lastRightSpeed) < 0;

    if(leftInverted || rightInverted) {
        release();
    }

    _applyPWM(leftSpeed, rightSpeed);
    _lastLeftSpeed = leftSpeed;
    _lastRightSpeed = rightSpeed;
}

void Drive::brake() {
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 100);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 100);
}

void Drive::release() {
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 0);
}

void Drive::_applyPWM(const int leftPWM, const int rightPWM) {
    if(leftPWM >= 0) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, float(leftPWM));
    }
    else {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, float(-leftPWM));
    }

    if(rightPWM >= 0) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, float(rightPWM));
    }
    else {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, float(-rightPWM));
    }
}