#include <Bluepad32.h>
#include <Receiver.hpp>
#include <algorithm>

Receiver *Receiver::instance = nullptr;

Receiver::Receiver() {
    instance = this;
}

void Receiver::init() {
    BP32.setup(&Receiver::onConnected, &Receiver::onDisconnected);
    Serial.println("[Receiver] Bluetooth Stack started. Looking for controllers...");
}

void Receiver::onConnected(ControllerPtr ctl) {
    ControllerProperties propriedades = ctl->getProperties();

    // Extrai o array de bytes do MAC Address de dentro da estrutura
    const uint8_t *mac = propriedades.btaddr;

    Serial.printf("[Receiver] ✅ Controller Connected! Address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1],
                  mac[2], mac[3], mac[4], mac[5]);

    if(instance) {
        instance->controller = ctl;
    }
}

void Receiver::onDisconnected(ControllerPtr ctl) {
    Serial.println("[Receiver] ❌ Controller Disconnected or signal lost!");
    if(instance) {
        instance->controller = nullptr;
    }
}

int Receiver::mapAxis(int rawValue) {
    if(abs(rawValue) < DEADZONE) {
        return 0;
    }

    // Converte a resolução e inverte o eixo Y do analógico (nativamente invertido no PS4)
    int percentage = -(rawValue * 100) / 511;

    // depois tem que substituir por umas matematica doida
    return percentage;
}

void Receiver::update() {
    BP32.update();

    int throttle = 0;
    int steer = 0;

    if(controller && controller->isConnected()) {
        // Eixo Y do Analógico Esquerdo: Acelerador (frente/trás)
        throttle = mapAxis(controller->axisY());

        // Eixo X do Analógico Direito: Direção (volante)
        steer = mapAxis(controller->axisRX());
    }

    Serial.printf("Analógico Esquerdo (Throttle): %d | Analógico Direito (Steer): %d\n", throttle, steer);
}
