#pragma once

#include "Receiver.hpp"

class ArmMotor;
class Drive;
class WeaponSystem;
class StatusLed;

/**
    @class RCMode
    @brief Actor responsible for managing all robot logic during manual control.
*/

class RCMode {
  public:
    /**
    @brief Initializes the RC mode and its subsystems.
    */
    void init();

    /**
    @brief Executes the manual control logic. Dependency injection: The FSM "lends" the motors for RCMode to drive.
    @param motores Reference to the Drive object to control the motors.
    @param braco   N20 do braço, nas setas cima/baixo.
    @param armas   Os servos do trator, em L1/L2 e R1/R2.
    @param led     Reference to the status LED strip — só usada pra piscar
                    quando o combo de polaridade dispara (ver handleMotorPolarity()).
    */
    void run(Drive &motores, ArmMotor &braco, WeaponSystem &armas, StatusLed &led);

    /**
    @brief Returns whether the controller is currently connected and recognized.
    */
    bool controllerConnected() const {
        return receptor.isConnected();
    }

  private:
    /**
    @brief The radio receiver instance. Composition: The radio is a private tool of RCMode.
    */
    Receiver receptor;

    void handleArm(ArmMotor &braco);

    void handleServos(WeaponSystem &armas);

    // Último millis() em que cada servo andou 1° — cadencia o "andar
    // enquanto segura" em SERVO_MS_PER_DEG, independente da taxa do loop.
    unsigned long _servoLastStep[2] = {0, 0};

    // Segurar Start por um tempo cicla a config de polaridade dos motores
    // sem precisar do site — ver .cpp.
    void handleMotorPolarity(Drive &motores, StatusLed &led);

    // 0 = Start não está sendo segurado (ou acabou de soltar). Setado no
    // instante em que Start desce, comparado contra millis() pra medir
    // quanto tempo já ficou segurado.
    unsigned long _polarityHoldStart = 0;

    // Falso logo depois de disparar — precisa soltar Start pra rearmar, senão
    // continuar segurando além do tempo dispararia cyclePolarity() todo
    // frame.
    bool _polarityArmed = true;
};