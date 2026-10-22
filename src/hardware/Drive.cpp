#include "Drive.hpp"
#include <Arduino.h>
#include <driver/mcpwm.h>

// Parece que existe uma biblioteca mais moderna pra isso e vou ter que fazer a troca

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

    // O timer default é de 1 MHz: a 20 kHz isso dá só 50 ticks por período,
    // ou seja, o duty anda de 2 em 2% (41 e 42 viram a mesma coisa). Com o
    // timer a 10 MHz são 500 ticks. Precisa vir antes do mcpwm_init, que é
    // quando o driver aplica os prescalers.
    mcpwm_group_set_resolution(MCPWM_UNIT_0, 10000000);
    mcpwm_timer_set_resolution(MCPWM_UNIT_0, MCPWM_TIMER_0, 10000000);
    mcpwm_timer_set_resolution(MCPWM_UNIT_0, MCPWM_TIMER_1, 10000000);

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

    // Corrige motor invertido / lados trocados aqui, no único ponto por onde
    // toda velocidade passa — nenhuma estratégia/modo precisa saber que isso
    // existe. Ver MotorPolarity.hpp pro porquê disso não mexer em pino nenhum.
    _polarity.apply(leftSpeed, rightSpeed);

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

// Slow decay: uma entrada fica em 1 e a outra recebe o PWM invertido. Na A4950
// IN1=1/IN2=0 aciona e IN1=1/IN2=1 freia, então no tempo "desligado" do pulso
// o motor fica em freio (corrente recircula) em vez de solto. O jeito antigo
// (PWM numa entrada, 0 na outra) alternava aciona/coast: a corrente despencava
// a cada ciclo e o motor perdia força nas velocidades intermediárias.
// Por isso a escala mudou — um mesmo valor de velocidade empurra mais que antes.
void Drive::_applyPWM(const int leftPWM, const int rightPWM) {
    if(leftPWM >= 0) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, float(100 - leftPWM));
    }
    else {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, float(100 + leftPWM));
    }

    if(rightPWM >= 0) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 100);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, float(100 - rightPWM));
    }
    else {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 100);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, float(100 + rightPWM));
    }
}