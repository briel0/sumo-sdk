#include "Receiver.hpp"
#include <esp_mac.h>

Receiver *Receiver::instance = nullptr;

Receiver::Receiver() {
    instance = this;
}

void Receiver::init() {
    BP32.setup(&Receiver::onConnected, &Receiver::onDisconnected);
    Serial.println("[Receiver] Bluetooth Stack started. Looking for controllers...");
}

void Receiver::onConnected(ControllerPtr ctl) {
    const uint8_t *incomingMac = ctl->getProperties().btaddr;

    if(instance->isPairingMode) {
        Serial.printf("[Receiver] ✅ PILOTO OFICIAL REGISTRADO! MAC: " MACSTR "\n", MAC2STR(incomingMac));
        memcpy(instance->savedMac, incomingMac, 6);
        instance->isPairingMode = false;
        instance->controller = ctl;
    }
    else {
        if(memcmp(incomingMac, instance->savedMac, 6) == 0) {
            Serial.println("[Receiver] ✅ Piloto oficial retornou. Conexão restabelecida.");
            instance->controller = ctl;
        }
        else {
            Serial.printf("[Receiver] ❌ ALERTA! Invasor bloqueado. MAC: " MACSTR "\n", MAC2STR(incomingMac));
            ctl->disconnect();
        }
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

    static unsigned long lastPrint = 0;
    if(millis() - lastPrint > 100) {
        Serial.printf("RT: %d | LT: %d | StickX: %d\n", rightTriggerVal, leftTriggerVal, leftStickXVal);
        lastPrint = millis();
    }
}

void Receiver::updateAxes() {
    int rawRight = controller->throttle();
    int rawLeft = controller->brake();
    int rawStick = controller->axisX();

    rightTriggerVal = (rawRight < TRIGGER_DEADZONE) ? 0 : (rawRight * 100) / 1023;
    leftTriggerVal = (rawLeft < TRIGGER_DEADZONE) ? 0 : (rawLeft * 100) / 1023;

    if(abs(rawStick) < STICKER_DEADZONE) {
        leftStickXVal = 0;
    }
    else {
        leftStickXVal = (rawStick * 100) / 511;
        leftStickXVal = constrain(leftStickXVal, -100, 100);
    }
}

void Receiver::updateButtons() {

    uint8_t currentDpad = controller->dpad();
    dpadUpFlag = (currentDpad & MASK_DPAD_UP) && !(lastDpad & MASK_DPAD_UP);
    dpadDownFlag = (currentDpad & MASK_DPAD_DOWN) && !(lastDpad & MASK_DPAD_DOWN);
    lastDpad = currentDpad;

    uint16_t currentBtns = controller->buttons();

    bool currentCircle = currentBtns & MASK_BTN_CIRCLE;
    circleFlag = currentCircle && !lastCircle;
    lastCircle = currentCircle;

    bool currentCross = currentBtns & MASK_BTN_CROSS;
    crossFlag = currentCross && !lastCross;
    lastCross = currentCross;

    bool currentSquare = currentBtns & MASK_BTN_SQUARE;
    squareFlag = currentSquare && !lastSquare;
    lastSquare = currentSquare;

    bool currentTriangle = currentBtns & MASK_BTN_TRIANGLE;
    triangleFlag = currentTriangle && !lastTriangle;
    lastTriangle = currentTriangle;
}

void Receiver::applyFailsafe() {
    rightTriggerVal = 0;
    leftTriggerVal = 0;
    leftStickXVal = 0;
    lastDpad = 0;
    lastCircle = false;
    lastCross = false;
    lastSquare = false;
    lastTriangle = false;
    dpadUpFlag = false;
    dpadDownFlag = false;
    circleFlag = false;
    crossFlag = false;
    squareFlag = false;
    triangleFlag = false;
}