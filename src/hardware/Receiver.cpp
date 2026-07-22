#include "Receiver.hpp"
#include <esp_mac.h>

// TO DO: REMOVER PREFS

Receiver *Receiver::instance = nullptr;

Receiver::Receiver() {
    instance = this;
}

void Receiver::init() {
    // Sempre limpa o cache — garante que o BTstack não tenta reconectar
    // com controles de sessões anteriores antes de você escolher qual aceitar
    BP32.forgetBluetoothKeys();

    isPairingMode = true;

    BP32.setup(&Receiver::onConnected, &Receiver::onDisconnected);
    Serial.println("[Receiver] Bluetooth Stack started. Looking for controllers...");
}

void Receiver::lockToSavedController() {
    isPairingMode = false;
    Serial.println("[Receiver] SECURE MODE ACTIVATED: Only the owner can connect.");
}

void Receiver::openForNewController() {
    isPairingMode = true;

    if(controller && controller->isConnected()) {
        controller->disconnect();
        controller = nullptr;
    }
    Serial.println("[Receiver] OPEN MODE: Waiting to pair a NEW controller...");
}

void Receiver::onConnected(ControllerPtr ctl) {
    if(instance->isPairingMode) {
        instance->controller = ctl;
        instance->isPairingMode = false; // trava — só esse controle agora
        Serial.printf("[Receiver] ✅ Controle aceito. MAC: " MACSTR "\n", MAC2STR(ctl->getProperties().btaddr));
    }
    else {
        Serial.println("[Receiver] ❌ Segundo controle rejeitado.");
        ctl->disconnect();
    }
}

void Receiver::onDisconnected(ControllerPtr ctl) {
    Serial.println("[Receiver] ❌ Controller Disconnected or signal lost!");
    if(instance) {
        instance->controller = nullptr;
    }
}

void Receiver::update() {
    BP32.update();

    if(controller && controller->isConnected()) {
        updateAxes();
        updateButtons();
    }
    else {
        applyFailsafe();
    }

    /*
    // Opcional: Descomente para debugar os eixos na serial
    static unsigned long lastPrint = 0;
    if(millis() - lastPrint > 2000) {
        Serial.printf("LX: %d | LY: %d | RX: %d | RY: %d | LT: %d | RT: %d\n", leftStickXVal, leftStickYVal,
                      rightStickXVal, rightStickYVal, leftTriggerVal, rightTriggerVal);
        lastPrint = millis();
    }
    */
}

void Receiver::updateAxes() {
    int rawLT = controller->brake();
    int rawRT = controller->throttle();
    int rawLX = controller->axisX();
    int rawLY = controller->axisY();
    int rawRX = controller->axisRX();
    int rawRY = controller->axisRY();

    // Mapeamento idêntico à versão mais recente (compatível com os getters do RCMode)
    leftTriggerVal = (rawLT < TRIGGER_DEADZONE) ? 0 : (rawLT * 100) / 1023;
    rightTriggerVal = (rawRT < TRIGGER_DEADZONE) ? 0 : (rawRT * 100) / 1023;

    leftStickXVal = (abs(rawLX) < STICKER_DEADZONE) ? 0 : constrain((rawLX * 100) / 511, -100, 100);
    leftStickYVal = (abs(rawLY) < STICKER_DEADZONE) ? 0 : constrain((rawLY * 100) / 511, -100, 100);
    rightStickXVal = (abs(rawRX) < STICKER_DEADZONE) ? 0 : constrain((rawRX * 100) / 511, -100, 100);
    rightStickYVal = (abs(rawRY) < STICKER_DEADZONE) ? 0 : constrain((rawRY * 100) / 511, -100, 100);
}

void Receiver::updateButtons() {
    uint8_t currentDpad = controller->dpad();
    dpadUpFlag = (currentDpad & DPAD_UP) && !(lastDpad & DPAD_UP);
    dpadDownFlag = (currentDpad & DPAD_DOWN) && !(lastDpad & DPAD_DOWN);
    dpadLeftFlag = (currentDpad & DPAD_LEFT) && !(lastDpad & DPAD_LEFT);
    dpadRightFlag = (currentDpad & DPAD_RIGHT) && !(lastDpad & DPAD_RIGHT);
    lastDpad = currentDpad;

    uint16_t currentBtns = controller->buttons();

    bool currentCircle = currentBtns & BUTTON_B;
    circleFlag = currentCircle && !lastCircle;
    lastCircle = currentCircle;

    bool currentCross = currentBtns & BUTTON_A;
    crossFlag = currentCross && !lastCross;
    lastCross = currentCross;

    bool currentSquare = currentBtns & BUTTON_X;
    squareFlag = currentSquare && !lastSquare;
    lastSquare = currentSquare;

    bool currentTriangle = currentBtns & BUTTON_Y;
    triangleFlag = currentTriangle && !lastTriangle;
    lastTriangle = currentTriangle;

    bool currentR3 = currentBtns & BUTTON_THUMB_R;
    r3Flag = currentR3 && !lastR3;
    lastR3 = currentR3;
}

void Receiver::applyFailsafe() {
    leftTriggerVal = 0;
    rightTriggerVal = 0;
    leftStickXVal = 0;
    leftStickYVal = 0;
    rightStickXVal = 0;
    rightStickYVal = 0;

    lastDpad = 0;
    lastCircle = false;
    lastCross = false;
    lastSquare = false;
    lastTriangle = false;

    dpadUpFlag = false;
    dpadDownFlag = false;
    dpadLeftFlag = false;
    dpadRightFlag = false;
    circleFlag = false;
    crossFlag = false;
    squareFlag = false;
    triangleFlag = false;

    lastR3 = false;
    r3Flag = false;
}