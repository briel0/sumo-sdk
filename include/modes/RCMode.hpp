#pragma once

#include "MotionPlayer.hpp"
#include "Receiver.hpp"

class Drive;
class WeaponSystem;

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
    */
    void run(Drive &motores, WeaponSystem &armas);

    /**
    @brief Returns whether the controller is currently connected and recognized.
    */
    bool controllerConnected() const {
        return receptor.isConnected();
    }

    void disconnectController() {
        receptor.disconnect();
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
};