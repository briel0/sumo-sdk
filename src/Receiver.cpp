#include "Receiver.hpp"
#include <esp_mac.h>

Receiver *Receiver::instance = nullptr;

Receiver::Receiver() {
    instance = this;
}

void Receiver::init() {
    pinMode(0, INPUT_PULLUP);

    prefs.begin("radio", false);

    prefs.getBytes("mac_dono", savedMac, 6);

    isPairingMode = true; // always open

    /*
    if(digitalRead(0) == LOW) {
        Serial.println("[Receiver] BOOT button detected! Opening for NEW controller...");
        isPairingMode = true;
    }
    else {
        Serial.println("[Receiver] Normal boot. Locking radio to the official pilot.");
        isPairingMode = false;
    }
    */

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
    const uint8_t *incomingMac = ctl->getProperties().btaddr;

    if(instance->isPairingMode) {
        Serial.printf("[Receiver] ✅ NEW OWNER ACCEPTED! MAC: " MACSTR "\n", MAC2STR(incomingMac));

        instance->prefs.putBytes("mac_dono", incomingMac, 6);
        memcpy(instance->savedMac, incomingMac, 6);

        instance->controller = ctl;

        instance->lockToSavedController();
    }
    else {
        if(memcmp(incomingMac, instance->savedMac, 6) == 0) {
            Serial.println("[Receiver] ✅ Official pilot recognized! Connection restored.");
            instance->controller = ctl;
        }
        else {
            Serial.printf("[Receiver] ❌ ALERT! Invader blocked. MAC: " MACSTR "\n", MAC2STR(incomingMac));
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

int Receiver::getThrottle() {
    return currentThrottle;
}

int Receiver::getSteer() {
    return currentSteer;
}

int Receiver::mapAxis(int rawValue) {
    if(abs(rawValue) < STICKER_DEADZONE) {
        return 0;
    }
    int percentage = (rawValue * 100) / 511;
    return constrain(percentage, -100, 100);
}

int Receiver::mapTrigger(int rawValue) {
    if(rawValue < TRIGGER_DEADZONE) {
        return 0;
    }
    int percentage = (rawValue * 100) / 1023;
    return constrain(percentage, 0, 100);
}

void Receiver::update() {
    BP32.update();

    if(controller && controller->isConnected()) {
        int forward = mapTrigger(controller->throttle());
        int reverse = mapTrigger(controller->brake());

        currentThrottle = forward - reverse;

        currentSteer = mapAxis(controller->axisX());
    }
    else {
        currentThrottle = 0;
        currentSteer = 0;
    }

    static unsigned long lastPrint = 0;
    if(millis() - lastPrint > 100) {
        Serial.printf("Throttle: %d | Steer: %d\n", currentThrottle, currentSteer);
        lastPrint = millis();
    }
}