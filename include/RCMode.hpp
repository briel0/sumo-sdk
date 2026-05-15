#pragma once

#include "Drive.hpp"
#include "Receiver.hpp"

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
    */
    void run(Drive &motores);

  private:
    /**
    @brief The radio receiver instance. Composition: The radio is a private tool of RCMode.
    */
    Receiver receptor;
};