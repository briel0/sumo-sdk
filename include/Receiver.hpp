#pragma once

#include <Arduino.h>
#include <Bluepad32.h>
#include <Preferences.h>

/**
    @class Receiver
    @brief Manages Bluetooth connections, analog input mapping, and the anti-hijacking security system for the robot's
   radio.
 */
class Receiver {
  public:
    /**
    @brief Constructs the Receiver object and establishes the static instance pointer.
    */
    Receiver();

    /**
    @brief Initializes the Bluepad32 Bluetooth stack and loads the saved MAC address from non-volatile memory.
    */
    void init();

    /**
    @brief Polls the controller inputs, processes deadzones, and handles the raw data stream.
    */
    void update();

    /**
    @brief Locks the radio receiver. Only the controller with the MAC address saved in Flash memory is allowed to
    connect.
    */
    void lockToSavedController();

    /**
    @brief Opens the radio receiver, forcibly disconnects the current controller, and accepts the first new controller
    that attempts to pair.
    */
    void openForNewController();

    /**
    @brief System callback executed by the Bluetooth stack when a controller attempts to connect.
    @param ctl Pointer to the controller object attempting the connection.
    */
    static void onConnected(ControllerPtr ctl);

    /**
    @brief System callback executed by the Bluetooth stack when a paired controller disconnects or loses its signal.
    @param ctl Pointer to the controller object that was disconnected.
    */
    static void onDisconnected(ControllerPtr ctl);

  private:
    /**
    @brief Pointer to the currently active and authorized Bluetooth controller.
    */
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
    @brief Interface for the ESP32's non-volatile storage (Flash) to save security data.
    */
    Preferences prefs;

    /**
    @brief Array storing the 6-byte MAC address of the officially paired controller.
    */
    uint8_t savedMac[6] = {0};

    /**
    @brief Security flag defining if the receiver is open to new pairings or locked to the saved MAC.
    */
    bool isPairingMode = false;
};