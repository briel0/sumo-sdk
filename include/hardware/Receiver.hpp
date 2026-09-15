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

    bool isConnected() const {
        return controller != nullptr && controller->isConnected();
    }

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
    bool crossHeld() const {
        return lastCross;
    }
    bool square() const {
        return squareFlag;
    }
    bool triangle() const {
        return triangleFlag;
    }

    bool r3() const {
        return r3Flag;
    }
    bool l3() const {
        return l3Flag;
    }

    bool r1() const {
        return r1Flag;
    }
    bool l1() const {
        return l1Flag;
    }

    // Nível, não borda — igual crossHeld(): "está segurando agora", não
    // "acabou de apertar". Start/Options não é usado em mais nada aqui, é
    // o modificador do combo pra ciclar a polaridade dos motores (ver
    // RCMode::handleMotorPolarity()).
    bool startHeld() const {
        return startFlag;
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

    bool r3Flag = false;
    bool lastR3 = false;

    bool l3Flag = false;
    bool lastL3 = false;

    bool r1Flag = false;
    bool lastR1 = false;

    bool l1Flag = false;
    bool lastL1 = false;

    bool startFlag = false;

    static Receiver *instance;
    uint8_t savedMac[6] = {0};
    bool isPairingMode = true;
    Preferences prefs;

    void updateAxes();
    void updateButtons();
    void applyFailsafe();
};