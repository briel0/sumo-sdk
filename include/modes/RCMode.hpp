#pragma once

#include "MotionPlayer.hpp"
#include "Receiver.hpp"

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
    @param armas   Reference to the WeaponSystem object to control the weapons.
    @param led     Reference to the status LED strip — só usada pra piscar
                    quando o combo de polaridade dispara (ver handleMotorPolarity()).
    */
    void run(Drive &motores, WeaponSystem &armas, StatusLed &led);

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
    MotionPlayer macroPlayer;

    /**
    @brief Flag to lock the automatic disarm logic, preventing the weapon from retracting.
    */
    bool _autoDisarmLocked = false;

    void handleWeapons(WeaponSystem &armas, int throttle, int steer);

    void handleMacros(Drive &motores, WeaponSystem &armas);

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