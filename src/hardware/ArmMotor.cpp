#include "ArmMotor.hpp"
#include <Arduino.h>
#include <driver/mcpwm.h>

ArmMotor::ArmMotor(int posPin, int negPin) {
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, posPin);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, negPin);

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 20000;
    pwm_config.cmpr_a = 0;
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &pwm_config);

    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);

    release();
}

void ArmMotor::setSpeed(int speed) {
    speed = constrain(speed, -100, 100);

    if(speed == _lastSpeed) {
        return;
    }

    if(speed == 0) {
        brake();
        _lastSpeed = 0;
        return;
    }

    // Mesma proteção do Drive: solta antes de inverter o sentido.
    if(speed * _lastSpeed < 0) {
        release();
    }

    if(speed > 0) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, float(speed));
    }
    else {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, float(-speed));
    }
    _lastSpeed = speed;
}

void ArmMotor::brake() {
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, 100);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, 100);
}

void ArmMotor::release() {
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, 0);
}
