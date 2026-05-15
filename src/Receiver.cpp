#include "Receiver.hpp"

Receiver *Receiver::instance = nullptr;

Receiver::Receiver() {
    instance = this;
}

void Receiver::init() {
    pinMode(0, INPUT_PULLUP);

    prefs.begin("radio", false);

    prefs.getBytes("mac_dono", savedMac, 6);

    if(digitalRead(0) == LOW) {
        Serial.println("[Receiver] Botão BOOT detectado! Abrindo para NOVO controle...");
        isPairingMode = true;
    }
    else {
        Serial.println("[Receiver] Boot normal. Trancando rádio para o piloto oficial.");
        isPairingMode = false;
    }

    BP32.setup(&Receiver::onConnected, &Receiver::onDisconnected);
    Serial.println("[Receiver] Bluetooth Stack started. Looking for controllers...");
}

void Receiver::lockToSavedController() {
    isPairingMode = false;
    Serial.println("[Receiver] MODO SEGURO ATIVADO: Apenas o dono pode conectar.");
}

void Receiver::openForNewController() {
    isPairingMode = true;

    if(controller && controller->isConnected()) {
        controller->disconnect();
        controller = nullptr;
    }
    Serial.println("[Receiver] MODO ABERTO: Aguardando pareamento de um NOVO controle...");
}

void Receiver::onConnected(ControllerPtr ctl) {
    const uint8_t *incomingMac = ctl->getProperties().btaddr;

    if(instance->isPairingMode) {
        Serial.printf("[Receiver] ✅ NOVO DONO ACEITO! MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", incomingMac[0],
                      incomingMac[1], incomingMac[2], incomingMac[3], incomingMac[4], incomingMac[5]);

        instance->prefs.putBytes("mac_dono", incomingMac, 6);
        memcpy(instance->savedMac, incomingMac, 6);

        instance->controller = ctl;

        instance->lockToSavedController();
    }
    else {
        if(memcmp(incomingMac, instance->savedMac, 6) == 0) {
            Serial.println("[Receiver] ✅ Piloto oficial reconhecido! Conexão restabelecida.");
            instance->controller = ctl;
        }
        else {
            Serial.printf("[Receiver] ❌ ALERTA! Invasor bloqueado. MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                          incomingMac[0], incomingMac[1], incomingMac[2], incomingMac[3], incomingMac[4],
                          incomingMac[5]);
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

    int throttle = 0;
    int steer = 0;

    if(controller && controller->isConnected()) {
        throttle = controller->axisY();
        steer = controller->axisRX();
    }

    // Filtro temporal: Evita o Watchdog Reset do ESP32 permitindo prints apenas a cada 100ms
    static unsigned long lastPrint = 0;
    if(millis() - lastPrint > 100) {
        Serial.printf("Throttle: %d | Steer: %d\n", throttle, steer);
        lastPrint = millis();
    }
}