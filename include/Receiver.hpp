#pragma once

#include <Arduino.h>
#include <Bluepad32.h>
#include <algorithm>

/**
    @class Receiver
    @brief Handles Bluetooth connection, input mapping, and safety fail-safes for wireless controllers.
*/

class Receiver {
  public:
    /**
    @brief Constructs the Receiver object and initializes the static instance pointer for the OS.
    */
    Receiver();

    /**
    @brief Initializes the Bluepad32 Bluetooth stack and registers connection callbacks.
    */
    void init();

    /**
    @brief Polls the controller inputs, processes deadzones, and sends drive commands to the robot.
    @param robo Reference to the active Robo instance that will receive movement commands.
    */
    void update();

    /**
    @brief System callback executed by FreeRTOS when a Bluetooth controller pairs with the ESP32.
    @param ctl Pointer to the controller object containing hardware and input states.
    */
    static void onConnected(ControllerPtr ctl);

    /**
    @brief System callback executed by FreeRTOS when a paired controller disconnects or loses signal.
    @param ctl Pointer to the controller object that was disconnected.
    */
    static void onDisconnected(ControllerPtr ctl);

  private:
    ControllerPtr controller = nullptr;

    /**
    @brief Raw input value threshold below which joystick movements are ignored to prevent drift.
    */
    static constexpr int DEADZONE = 40;

    /**
    @brief Static self-pointer used by C-style OS callbacks to access this object instance in RAM.
    */
    static Receiver *instance;

    /**
    @brief Converts raw analog axis inputs to clean percentage values.
    @param rawValue Raw axis value from the controller (-511 to +511).
    @return Normalized percentage value (-100 to +100).
    */
    int mapAxis(int rawValue);
};