#pragma once

#include <Arduino.h>
#include <Bluepad32.h>

class Receiver {
  public:
    Receiver();
    void init();
    void update();

    void disconnect();

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

    // MAC do controle aceito neste power-cycle. Vem de Config::CONTROLLER_MAC
    // (allowlist em tempo de compilação). Se o perfil não fixou um MAC
    // (tudo-zero), entra em modo descoberta: aceita o primeiro controle que
    // conectar, trava nele pelo resto da sessão e imprime o MAC no serial pra
    // você fixar no perfil. Uma vez que o perfil tem MAC, o pareamento é
    // determinístico — só aquele controle conecta, eliminando o cross-pairing
    // entre dois robôs nossos pareando ao mesmo tempo.
    uint8_t savedMac[6] = {0};
    bool    discoveryMode = true;

    static bool macIsZero(const uint8_t *mac);

    void updateAxes();
    void updateButtons();
    void applyFailsafe();
};