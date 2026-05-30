#pragma once

#include <Arduino.h>
#include <Bluepad32.h>
#include <Preferences.h>

class Receiver {
  public:
    Receiver();
    void init();
    void update();

    void lockToSavedController();
    void openForNewController();

    static void onConnected(ControllerPtr ctl);
    static void onDisconnected(ControllerPtr ctl);

    int leftTrigger() const {
        return leftTriggerVal;
    }
    int rightTrigger() const {
        return rightTriggerVal;
    }

    int leftStickX() const {
        return leftStickXVal;
    }
    int leftStickY() const {
        return leftStickYVal;
    }
    int rightStickX() const {
        return rightStickXVal;
    }
    int rightStickY() const {
        return rightStickYVal;
    }

    bool dpadUp() const {
        return dpadUpFlag;
    }
    bool dpadDown() const {
        return dpadDownFlag;
    }
    bool dpadLeft() const {
        return dpadLeftFlag;
    }
    bool dpadRight() const {
        return dpadRightFlag;
    }

    bool circle() const {
        return circleFlag;
    }
    bool cross() const {
        return crossFlag;
    }
    bool square() const {
        return squareFlag;
    }
    bool triangle() const {
        return triangleFlag;
    }

  private:
    ControllerPtr controller = nullptr;
    static constexpr int STICKER_DEADZONE = 40;
    static constexpr int TRIGGER_DEADZONE = 15;

    int leftTriggerVal = 0;
    int rightTriggerVal = 0;
    int leftStickXVal = 0;
    int leftStickYVal = 0;
    int rightStickXVal = 0;
    int rightStickYVal = 0;

    bool dpadUpFlag = false;
    bool dpadDownFlag = false;
    bool dpadLeftFlag = false;
    bool dpadRightFlag = false;
    bool circleFlag = false;
    bool crossFlag = false;
    bool squareFlag = false;
    bool triangleFlag = false;

    uint8_t lastDpad = 0;
    bool lastCircle = false;
    bool lastCross = false;
    bool lastSquare = false;
    bool lastTriangle = false;

    static Receiver *instance;
    uint8_t savedMac[6] = {0};
    bool isPairingMode = true;
    Preferences prefs;

    void updateAxes();
    void updateButtons();
    void applyFailsafe();
};