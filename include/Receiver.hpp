#pragma once

#include <Arduino.h>
#include <Bluepad32.h>

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
    @brief System callback executed by the Bluetooth stack when a controller attempts to connect.
    @param ctl Pointer to the controller object attempting the connection.
    */
    static void onConnected(ControllerPtr ctl);

    /**
    @brief System callback executed by the Bluetooth stack when a paired controller disconnects or loses its signal.
    @param ctl Pointer to the controller object that was disconnected.
    */
    static void onDisconnected(ControllerPtr ctl);

    int rightTrigger() const {
        return rightTriggerVal;
    }
    int leftTrigger() const {
        return leftTriggerVal;
    }
    int leftStickX() const {
        return leftStickXVal;
    }

    bool dpadUp() const {
        return dpadUpFlag;
    }
    bool dpadDown() const {
        return dpadDownFlag;
    }
    bool Circle() const {
        return circleFlag;
    }

    bool Cross() const {
        return crossFlag;
    }

    bool Square() const {
        return squareFlag;
    }

    bool Triangle() const {
        return triangleFlag;
    }

  private:
    ControllerPtr controller = nullptr;
    static constexpr int STICKER_DEADZONE = 40;
    static constexpr int TRIGGER_DEADZONE = 15;

    uint8_t lastDpad = 0;
    bool lastCircle = false;
    bool lastCross = false;
    bool lastSquare = false;
    bool lastTriangle = false;

    bool dpadUpFlag = false;
    bool dpadDownFlag = false;
    bool circleFlag = false;
    bool crossFlag = false;
    bool squareFlag = false;
    bool triangleFlag = false;

    int rightTriggerVal = 0;
    int leftTriggerVal = 0;

    int leftStickXVal = 0;

    static Receiver *instance;
    uint8_t savedMac[6] = {0};
    bool isPairingMode = true;

    static constexpr uint8_t MASK_DPAD_UP = 0x01;
    static constexpr uint8_t MASK_DPAD_DOWN = 0x02;

    static constexpr uint16_t MASK_BTN_CROSS = 0x0001;    // X do PS4 / A do Xbox
    static constexpr uint16_t MASK_BTN_CIRCLE = 0x0002;   // Bolinha do PS4 / B do Xbox
    static constexpr uint16_t MASK_BTN_SQUARE = 0x0004;   // Quadrado do PS4 / X do Xbox
    static constexpr uint16_t MASK_BTN_TRIANGLE = 0x0008; // Triângulo do PS4 / Y do Xbox

    void updateAxes();
    void updateButtons();
    void applyFailsafe();
};