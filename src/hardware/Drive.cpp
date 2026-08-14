#include "Drive.hpp"
#include <Arduino.h>

#if !defined(CONFIG_IDF_TARGET_ESP32C3)
// ============================================================================
// ESP32 clássico (e S3): usa o periférico MCPWM diretamente.
// ============================================================================
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

#else
// ============================================================================
// ESP32-C3 (Fumacinha RC): sem MCPWM — 4 canais LEDC via ESP32PWM.
//
// A mesma ponte H de 2 pinos por motor: avançar = IN1 em PWM / IN2 em 0; ré =
// IN1 em 0 / IN2 em PWM; frear = os dois pinos em 100% (curto); soltar = os dois
// em 0. Os 4 canais aqui + os 2 dos servos ocupam os 6 canais LEDC do C3.
//
// O attach dos canais é preguiçoso (primeiro setSpeed, já no loop) de propósito:
// mexer no LEDC durante a construção estática do objeto global 'motores' — antes
// do setup() — é frágil. Como os servos prendem seus canais no setup(), deixar o
// Drive prender no loop garante ordem previsível na alocação dos 6 canais.
// ============================================================================

Drive::Drive(int rightPosPin, int rightNegPin, int leftPosPin, int leftNegPin)
    : _rightPosPin(rightPosPin), _rightNegPin(rightNegPin), _leftPosPin(leftPosPin), _leftNegPin(leftNegPin) {
    _lastLeftSpeed  = 0;
    _lastRightSpeed = 0;
}

void Drive::_ensureAttached() {
    if(_attached) {
        return;
    }
    _attached = true; // antes do release() abaixo, que reentra aqui

    const double FREQ_HZ    = 20000.0;
    const uint8_t RES_BITS  = 8;
    _pwmRightPos.attachPin(_rightPosPin, FREQ_HZ, RES_BITS);
    _pwmRightNeg.attachPin(_rightNegPin, FREQ_HZ, RES_BITS);
    _pwmLeftPos.attachPin(_leftPosPin, FREQ_HZ, RES_BITS);
    _pwmLeftNeg.attachPin(_leftNegPin, FREQ_HZ, RES_BITS);

    release();
}

void Drive::_drivePair(ESP32PWM &posCh, ESP32PWM &negCh, int speed) {
    if(speed >= 0) {
        negCh.writeScaled(0.0);
        posCh.writeScaled(speed / 100.0);
    }
    else {
        posCh.writeScaled(0.0);
        negCh.writeScaled(-speed / 100.0);
    }
}

void Drive::setSpeed(int leftSpeed, int rightSpeed) {
    _ensureAttached();

    leftSpeed  = constrain(leftSpeed, -100, 100);
    rightSpeed = constrain(rightSpeed, -100, 100);

    if(leftSpeed == _lastLeftSpeed && rightSpeed == _lastRightSpeed) {
        return;
    }

    if(leftSpeed == 0 && rightSpeed == 0) {
        brake();
        delayMicroseconds(25);
        _lastLeftSpeed  = 0;
        _lastRightSpeed = 0;
        return;
    }

    bool leftInverted  = (leftSpeed * _lastLeftSpeed) < 0;
    bool rightInverted = (rightSpeed * _lastRightSpeed) < 0;

    if(leftInverted || rightInverted) {
        release();
    }

    _applyPWM(leftSpeed, rightSpeed);
    _lastLeftSpeed  = leftSpeed;
    _lastRightSpeed = rightSpeed;
}

void Drive::brake() {
    _ensureAttached();
    _pwmLeftPos.writeScaled(1.0);
    _pwmLeftNeg.writeScaled(1.0);
    _pwmRightPos.writeScaled(1.0);
    _pwmRightNeg.writeScaled(1.0);
}

void Drive::release() {
    _pwmLeftPos.writeScaled(0.0);
    _pwmLeftNeg.writeScaled(0.0);
    _pwmRightPos.writeScaled(0.0);
    _pwmRightNeg.writeScaled(0.0);
}

void Drive::_applyPWM(const int leftPWM, const int rightPWM) {
    _drivePair(_pwmLeftPos, _pwmLeftNeg, leftPWM);
    _drivePair(_pwmRightPos, _pwmRightNeg, rightPWM);
}

#endif
